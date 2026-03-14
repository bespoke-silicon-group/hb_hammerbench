#include <bsg_cuda_lite_barrier.h>
#include <bsg_manycore.h>

#define INT4_MAX 7
#define INT4_MIN (-8)
#define MAX_ROW_WEIGHT 68

static int clamp4(int x) {
  if (x > INT4_MAX) return INT4_MAX;
  if (x < INT4_MIN) return INT4_MIN;
  return x;
}

static int iabs(int x) { return (x < 0) ? -x : x; }

// Exact signed truncation-toward-zero division by 20 using only mul
// (lower 32 bits). The vanilla core lacks mulh, so we use:
//   (x * 205 + bias) >> 12,  bias = 4099 for negative x, 0 otherwise.
// Exact for |x| <= 1009; our damping inputs satisfy |x| <= 150.
static inline int div20(int x) {
  int bias = (x >> 31) & 4099;
  return (x * 205 + bias) >> 12;
}

#ifdef WARM_CACHE
__attribute__((noinline)) static void warmup(int* B, int* r, int* R_arr,
                                             int* R_new_buf, int bg_size,
                                             int cols, int num_edges_z,
                                             int rnew_size) {
  int stride = bsg_tiles_X * bsg_tiles_Y * CACHE_LINE_WORDS;

  for (int i = __bsg_id * CACHE_LINE_WORDS; i < bg_size; i += stride)
    asm volatile("lw x0, %[p]" ::[p] "m"(B[i]));

  for (int i = __bsg_id * CACHE_LINE_WORDS; i < cols; i += stride)
    asm volatile("lw x0, %[p]" ::[p] "m"(r[i]));

  for (int i = __bsg_id * CACHE_LINE_WORDS; i < num_edges_z; i += stride)
    asm volatile("sw x0, %[p]" ::[p] "m"(R_arr[i]));

  for (int i = __bsg_id * CACHE_LINE_WORDS; i < rnew_size; i += stride)
    asm volatile("sw x0, %[p]" ::[p] "m"(R_new_buf[i]));

  bsg_fence();
}
#endif

// Hybrid z + edge parallel layered min-sum LDPC decoder.
//
// Optimizations:
//   1. DMEM-resident L with remote pointer access.
//   2. R_arr transposed layout for sequential edge reads.
//   3. Per-layer barriers dropped (fence only).
//   4. All runtime power-of-2 divisions/modulos replaced with shift/mask.
//      z, z_per_tile, tiles_per_z are always powers of 2.
//   5. Damping division by 20 replaced with mul-based sequence (no mulh).
//   6. Modular sp increment via bitmask (branchless).
//
extern "C" __attribute__((noinline)) int kernel_LDPC(int* B, int* r, int* L,
                                                     int* R_arr, int* R_new_buf,
                                                     int* hard_out, int bg_rows,
                                                     int bg_cols, int z,
                                                     int num_edges,
                                                     int* partial_buf) {
  bsg_barrier_tile_group_init();

  int cols = bg_cols * z;
  int num_tiles = bsg_tiles_X * bsg_tiles_Y;

  int tiles_per_z, z_per_tile, z_group, edge_rank;
  int my_z_start, my_z_end;

  if (z >= num_tiles) {
    tiles_per_z = 1;
    z_per_tile = z / num_tiles;
    z_group = __bsg_id;
    edge_rank = 0;
    my_z_start = __bsg_id * z_per_tile;
    my_z_end = my_z_start + z_per_tile;
  } else {
    tiles_per_z = num_tiles >> __builtin_ctz(z);
    z_per_tile = 1;
    int tpz_s = __builtin_ctz(tiles_per_z);
    z_group = __bsg_id >> tpz_s;
    edge_rank = __bsg_id & (tiles_per_z - 1);
    my_z_start = z_group;
    my_z_end = z_group + 1;
  }

  int num_z_groups = (z < num_tiles) ? z : num_tiles;

  // Precomputed masks and shifts for power-of-2 division/modulo elimination.
  // z, z_per_tile, and tiles_per_z are always powers of 2.
  int z_mask     = z - 1;
  int zpt_shift  = __builtin_ctz(z_per_tile);
  int zpt_mask   = z_per_tile - 1;
  int tpz_shift  = __builtin_ctz(tiles_per_z);
  int tpz_mask   = tiles_per_z - 1;

  int L_local[bg_cols * z_per_tile];

#ifdef WARM_CACHE
  warmup(B, r, R_arr, R_new_buf, bg_rows * bg_cols, cols, num_edges * z,
         MAX_ROW_WEIGHT * z);
#endif
  bsg_barrier_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  if (edge_rank == 0) {
    for (int c = 0; c < bg_cols; c++)
      for (int p = my_z_start; p < my_z_end; p++)
        L_local[c * z_per_tile + (p - my_z_start)] = r[c * z + p];
  }

  {
    int r_total = num_edges * z;
    int chunk = r_total / num_tiles;
    int start = __bsg_id * chunk;
    int end = start + chunk;
    for (int i = start; i < end; i++) R_arr[i] = 0;
  }

  bsg_fence();
  bsg_barrier_tile_group_sync();

  int row_edge_start[46];
  {
    int cum = 0;
    for (int lay = 0; lay < bg_rows; lay++) {
      row_edge_start[lay] = cum;
      for (int c = 0; c < bg_cols; c++)
        if (B[lay * bg_cols + c] != -1) cum++;
    }
  }

  int max_iter = 100;
  int local_msgs[MAX_ROW_WEIGHT * z_per_tile];
  int tile_R_new[MAX_ROW_WEIGHT * z_per_tile];

  for (int it = 0; it < max_iter; it++) {
    for (int layer = 0; layer < bg_rows; layer++) {
      int conn_cols[MAX_ROW_WEIGHT];
      int conn_shifts[MAX_ROW_WEIGHT];
      int nc = 0;
      int edge_base = row_edge_start[layer];

      for (int c = 0; c < bg_cols; c++) {
        int bval = B[layer * bg_cols + c];
        if (bval != -1) {
          conn_cols[nc] = c;
          conn_shifts[nc] = bval;
          nc++;
        }
      }
      if (nc == 0) continue;

      int my_e_start, my_e_end;
      if (tiles_per_z > 1) {
        int base = nc >> tpz_shift;
        int remainder = nc & tpz_mask;
        my_e_start = edge_rank * base + (edge_rank < remainder ? edge_rank : remainder);
        my_e_end = my_e_start + base + (edge_rank < remainder ? 1 : 0);
      } else {
        my_e_start = 0;
        my_e_end = nc;
      }

      // ---- Phase 2: min-sum ----
      if (tiles_per_z > 1) {
        for (int p = my_z_start; p < my_z_end; p++) {
          int lp = p - my_z_start;
          int min1 = 127, min2 = 127, min1_pos = my_e_start, par = 1;

          for (int e = my_e_start; e < my_e_end; e++) {
            int col = conn_cols[e];
            int shift = conn_shifts[e];
            int inv_sp = (p - shift) & z_mask;

            int tzg = inv_sp >> zpt_shift;
            int tid = tzg * tiles_per_z;
            volatile int* rL = bsg_tile_group_remote_ptr(
                int, tid % bsg_tiles_X, tid / bsg_tiles_X,
                &L_local[col * z_per_tile + (inv_sp & zpt_mask)]);
            int L_val = *rL;

            int R_old = R_arr[p * num_edges + edge_base + e];
            int val = L_val - R_old;
            local_msgs[e * z_per_tile + lp] = val;

            int sign = (val >= 0) ? 1 : -1;
            par *= sign;
            int av = clamp4(iabs(val));
            if (av < min1) {
              min2 = min1; min1 = av; min1_pos = e;
            } else if (av < min2) {
              min2 = av;
            }
          }

          int idx = (p * tiles_per_z + edge_rank) * 4;
          partial_buf[idx + 0] = clamp4(min1);
          partial_buf[idx + 1] = clamp4(min2);
          partial_buf[idx + 2] = min1_pos;
          partial_buf[idx + 3] = par;
        }

        bsg_fence();
        bsg_barrier_tile_group_sync();

        for (int p = my_z_start; p < my_z_end; p++) {
          int winner = 0, best = 127;
          for (int er = 0; er < tiles_per_z; er++) {
            int idx = (p * tiles_per_z + er) * 4;
            if (partial_buf[idx] < best) {
              best = partial_buf[idx]; winner = er;
            }
          }

          int w_idx = (p * tiles_per_z + winner) * 4;
          int gmin1 = partial_buf[w_idx + 0];
          int gmin2 = partial_buf[w_idx + 1];
          int gmin1_pos = partial_buf[w_idx + 2];
          int gparity = 1;

          for (int er = 0; er < tiles_per_z; er++) {
            int idx = (p * tiles_per_z + er) * 4;
            gparity *= partial_buf[idx + 3];
            if (er != winner && partial_buf[idx] < gmin2)
              gmin2 = partial_buf[idx];
          }

          int lp = p - my_z_start;
          for (int e = my_e_start; e < my_e_end; e++) {
            int val = local_msgs[e * z_per_tile + lp];
            int sign = (val >= 0) ? 1 : -1;
            int out_sign = gparity * sign;
            int use_mag = (gmin1_pos == e) ? gmin2 : gmin1;
            if (use_mag < 0) use_mag = 0;
            tile_R_new[e * z_per_tile + lp] = out_sign * use_mag;
          }
        }
      } else {
        int nc2 = nc & ~1;
        for (int e = 0; e < nc2; e += 2) {
          int col0 = conn_cols[e], col1 = conn_cols[e + 1];
          int shift0 = conn_shifts[e], shift1 = conn_shifts[e + 1];
          int eidx0 = edge_base + e, eidx1 = edge_base + e + 1;
          int inv_sp0 = (my_z_start - shift0) & z_mask;
          int inv_sp1 = (my_z_start - shift1) & z_mask;

          for (int p = my_z_start; p < my_z_end; p++) {
            int lp = p - my_z_start;
            int tzg0 = inv_sp0 >> zpt_shift, tid0 = tzg0 * tiles_per_z;
            int tzg1 = inv_sp1 >> zpt_shift, tid1 = tzg1 * tiles_per_z;
            int lv0 = *bsg_tile_group_remote_ptr(int, tid0 % bsg_tiles_X, tid0 / bsg_tiles_X, &L_local[col0 * z_per_tile + (inv_sp0 & zpt_mask)]);
            int lv1 = *bsg_tile_group_remote_ptr(int, tid1 % bsg_tiles_X, tid1 / bsg_tiles_X, &L_local[col1 * z_per_tile + (inv_sp1 & zpt_mask)]);
            local_msgs[e * z_per_tile + lp] = lv0 - R_arr[p * num_edges + eidx0];
            local_msgs[(e + 1) * z_per_tile + lp] = lv1 - R_arr[p * num_edges + eidx1];
            inv_sp0 = (inv_sp0 + 1) & z_mask;
            inv_sp1 = (inv_sp1 + 1) & z_mask;
          }
        }
        for (int e = nc2; e < nc; e++) {
          int col = conn_cols[e];
          int shift = conn_shifts[e];
          int eidx = edge_base + e;
          int inv_sp = (my_z_start - shift) & z_mask;

          for (int p = my_z_start; p < my_z_end; p++) {
            int tzg = inv_sp >> zpt_shift;
            int tid = tzg * tiles_per_z;
            int L_val = *bsg_tile_group_remote_ptr(int, tid % bsg_tiles_X, tid / bsg_tiles_X, &L_local[col * z_per_tile + (inv_sp & zpt_mask)]);
            local_msgs[e * z_per_tile + (p - my_z_start)] = L_val - R_arr[p * num_edges + eidx];
            inv_sp = (inv_sp + 1) & z_mask;
          }
        }

        for (int p = my_z_start; p < my_z_end; p++) {
          int lp = p - my_z_start;
          int min1 = 127, min2 = 127;
          int min1_pos = 0;
          int parity = 1;

          for (int e = 0; e < nc; e++) {
            int val = local_msgs[e * z_per_tile + lp];
            int sign = (val >= 0) ? 1 : -1;
            parity *= sign;
            int av = clamp4(iabs(val));
            if (av < min1) {
              min2 = min1; min1 = av; min1_pos = e;
            } else if (av < min2) {
              min2 = av;
            }
          }
          min1 = clamp4(min1);
          min2 = clamp4(min2);

          for (int e = 0; e < nc; e++) {
            int val = local_msgs[e * z_per_tile + lp];
            int sign = (val >= 0) ? 1 : -1;
            int out_sign = parity * sign;
            int use_mag = (min1_pos == e) ? min2 : min1;
            if (use_mag < 0) use_mag = 0;
            tile_R_new[e * z_per_tile + lp] = out_sign * use_mag;
          }
        }
      }

      // ---- Phase 3: damp R_arr, remote-store L_new ----
      {
        int ne = my_e_end - my_e_start;
        int ne2 = ne & ~1;
        for (int ei = 0; ei < ne2; ei += 2) {
          int e0 = my_e_start + ei, e1 = my_e_start + ei + 1;
          int col0 = conn_cols[e0], col1 = conn_cols[e1];
          int eidx0 = edge_base + e0, eidx1 = edge_base + e1;
          int sp0 = (my_z_start - conn_shifts[e0]) & z_mask;
          int sp1 = (my_z_start - conn_shifts[e1]) & z_mask;

          for (int q = my_z_start; q < my_z_end; q++) {
            int lp = q - my_z_start;
            int ro0 = R_arr[q * num_edges + eidx0];
            int ro1 = R_arr[q * num_edges + eidx1];
            int rn0 = tile_R_new[e0 * z_per_tile + lp];
            int rn1 = tile_R_new[e1 * z_per_tile + lp];
            R_arr[q * num_edges + eidx0] = div20(ro0 * 7 + rn0 * 13 + 10);
            R_arr[q * num_edges + eidx1] = div20(ro1 * 7 + rn1 * 13 + 10);
            int lv0 = local_msgs[e0 * z_per_tile + lp] + rn0;
            int lv1 = local_msgs[e1 * z_per_tile + lp] + rn1;
            int tzg0 = sp0 >> zpt_shift, tid0 = tzg0 * tiles_per_z;
            int tzg1 = sp1 >> zpt_shift, tid1 = tzg1 * tiles_per_z;
            *bsg_tile_group_remote_ptr(int, tid0 % bsg_tiles_X, tid0 / bsg_tiles_X, &L_local[col0 * z_per_tile + (sp0 & zpt_mask)]) = lv0;
            *bsg_tile_group_remote_ptr(int, tid1 % bsg_tiles_X, tid1 / bsg_tiles_X, &L_local[col1 * z_per_tile + (sp1 & zpt_mask)]) = lv1;
            sp0 = (sp0 + 1) & z_mask;
            sp1 = (sp1 + 1) & z_mask;
          }
        }
        for (int ei = ne2; ei < ne; ei++) {
          int e = my_e_start + ei;
          int col = conn_cols[e];
          int eidx = edge_base + e;
          int sp = (my_z_start - conn_shifts[e]) & z_mask;

          for (int q = my_z_start; q < my_z_end; q++) {
            int lp = q - my_z_start;
            int R_old = R_arr[q * num_edges + eidx];
            int R_new_val = tile_R_new[e * z_per_tile + lp];
            R_arr[q * num_edges + eidx] = div20(R_old * 7 + R_new_val * 13 + 10);
            int L_new = local_msgs[e * z_per_tile + lp] + R_new_val;
            int tzg = sp >> zpt_shift;
            int tid = tzg * tiles_per_z;
            *bsg_tile_group_remote_ptr(int, tid % bsg_tiles_X, tid / bsg_tiles_X, &L_local[col * z_per_tile + (sp & zpt_mask)]) = L_new;
            sp = (sp + 1) & z_mask;
          }
        }
      }

      bsg_fence();
    }

    bsg_barrier_tile_group_sync();

    // Convergence check: all tiles participate, layers striped by edge_rank
    {
      int my_fail = 0;
      int layer_stride = (tiles_per_z > 1) ? tiles_per_z : 1;
      int layer_start = (tiles_per_z > 1) ? edge_rank : 0;
      for (int layer = layer_start; layer < bg_rows && !my_fail; layer += layer_stride) {
        int* B_row = &B[layer * bg_cols];
        for (int p = my_z_start; p < my_z_end && !my_fail; p++) {
          int syndrome = 0;
          for (int c = 0; c < bg_cols; c += 4) {
            int s0 = B_row[c], s1 = B_row[c + 1], s2 = B_row[c + 2], s3 = B_row[c + 3];
            if (s0 != -1) {
              int sp = (p + s0) & z_mask; int tzg = sp >> zpt_shift; int tid = tzg * tiles_per_z;
              syndrome ^= (*bsg_tile_group_remote_ptr(int, tid % bsg_tiles_X, tid / bsg_tiles_X, &L_local[c * z_per_tile + (sp & zpt_mask)]) < 0) ? 1 : 0;
            }
            if (s1 != -1) {
              int sp = (p + s1) & z_mask; int tzg = sp >> zpt_shift; int tid = tzg * tiles_per_z;
              syndrome ^= (*bsg_tile_group_remote_ptr(int, tid % bsg_tiles_X, tid / bsg_tiles_X, &L_local[(c + 1) * z_per_tile + (sp & zpt_mask)]) < 0) ? 1 : 0;
            }
            if (s2 != -1) {
              int sp = (p + s2) & z_mask; int tzg = sp >> zpt_shift; int tid = tzg * tiles_per_z;
              syndrome ^= (*bsg_tile_group_remote_ptr(int, tid % bsg_tiles_X, tid / bsg_tiles_X, &L_local[(c + 2) * z_per_tile + (sp & zpt_mask)]) < 0) ? 1 : 0;
            }
            if (s3 != -1) {
              int sp = (p + s3) & z_mask; int tzg = sp >> zpt_shift; int tid = tzg * tiles_per_z;
              syndrome ^= (*bsg_tile_group_remote_ptr(int, tid % bsg_tiles_X, tid / bsg_tiles_X, &L_local[(c + 3) * z_per_tile + (sp & zpt_mask)]) < 0) ? 1 : 0;
            }
          }
          if (syndrome) my_fail = 1;
        }
      }
      partial_buf[__bsg_id] = my_fail;
    }

    bsg_fence();
    bsg_barrier_tile_group_sync();

    int converged = 1;
    for (int t = 0; t < num_tiles; t++) {
      if (partial_buf[t]) { converged = 0; break; }
    }

    if (__bsg_id == 0)
      bsg_printf("iter %d, parity=%d\n", it, !converged);

    if (converged) break;
  }

  if (edge_rank == 0) {
    for (int c = 0; c < bg_cols; c++)
      for (int p = my_z_start; p < my_z_end; p++)
        hard_out[c * z + p] = (L_local[c * z_per_tile + (p - my_z_start)] < 0) ? 1 : 0;
  }

  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_tile_group_sync();

  return 0;
}
