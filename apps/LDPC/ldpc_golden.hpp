#ifndef LDPC_GOLDEN_HPP
#define LDPC_GOLDEN_HPP

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define DAMPING 0.65
#define INT4_MAX 7
#define INT4_MIN (-8)

static double quantize4(double x) {
  x = floor(x);
  if (x > INT4_MAX) return INT4_MAX;
  if (x < INT4_MIN) return INT4_MIN;
  return x;
}

// Layered min-sum decoder

static int golden_decode(int* B, int* r, int bg_rows, int bg_cols, int z,
                         int max_iter, int* hard_out) {
  int cols = bg_cols * z;

  int num_edges = 0;
  int* edge_map = (int*)malloc(bg_rows * bg_cols * sizeof(int));
  for (int i = 0; i < bg_rows * bg_cols; i++)
    edge_map[i] = (B[i] != -1) ? num_edges++ : -1;

  double* L = (double*)malloc(cols * sizeof(double));
  for (int j = 0; j < cols; j++)
    L[j] = (double)r[j];

  double* R = (double*)calloc(num_edges * z, sizeof(double));

  // Scratch buffers (max connected cols per layer = bg_cols)
  double* local_msgs = (double*)malloc(bg_cols * z * sizeof(double));
  double* R_new = (double*)malloc(bg_cols * z * sizeof(double));

  int result_iter = max_iter;

  for (int it = 0; it < max_iter; it++) {
    for (int layer = 0; layer < bg_rows; layer++) {
      int conn_cols[68], conn_shifts[68], conn_edges[68];
      int nc = 0;
      for (int c = 0; c < bg_cols; c++) {
        int bval = B[layer * bg_cols + c];
        if (bval != -1) {
          conn_cols[nc] = c;
          conn_shifts[nc] = bval;
          conn_edges[nc] = edge_map[layer * bg_cols + c];
          nc++;
        }
      }
      if (nc == 0) continue;

      // Step 1: subtract old R from L, compute shifted local messages
      for (int e = 0; e < nc; e++) {
        int col = conn_cols[e];
        int shift = conn_shifts[e];
        int eidx = conn_edges[e];

        // np.roll(R, -shift): out[p] = R[(p+shift)%z]
        for (int p = 0; p < z; p++)
          L[col * z + p] -= R[eidx * z + ((p + shift) % z)];

        // np.roll(L_block, +shift): out[p] = L_block[(p-shift+z)%z]
        for (int p = 0; p < z; p++)
          local_msgs[e * z + p] = L[col * z + ((p - shift % z + z) % z)];
      }

      // Step 2: min-sum check node update (per z-position)
      for (int p = 0; p < z; p++) {
        double abs_vals[68];
        int signs[68];
        for (int e = 0; e < nc; e++) {
          double val = local_msgs[e * z + p];
          abs_vals[e] = quantize4(fabs(val));
          signs[e] = (val >= 0) ? 1 : -1;
        }

        double min1 = 1e9, min2 = 1e9;
        int min1_pos = 0;
        for (int e = 0; e < nc; e++) {
          if (abs_vals[e] < min1) {
            min2 = min1;
            min1 = abs_vals[e];
            min1_pos = e;
          } else if (abs_vals[e] < min2) {
            min2 = abs_vals[e];
          }
        }
        min1 = quantize4(min1);
        min2 = quantize4(min2);

        int parity = 1;
        for (int e = 0; e < nc; e++)
          parity *= signs[e];

        for (int e = 0; e < nc; e++) {
          int out_sign = parity * signs[e];
          double use_mag = (min1_pos == e) ? min2 : min1;
          if (use_mag < 0) use_mag = 0;
          R_new[e * z + p] = out_sign * use_mag;
        }
      }

      // Step 3: damp R, add R_new (undamped) back to L
      for (int e = 0; e < nc; e++) {
        int col = conn_cols[e];
        int shift = conn_shifts[e];
        int eidx = conn_edges[e];

        for (int p = 0; p < z; p++)
          R[eidx * z + p] =
              (1.0 - DAMPING) * R[eidx * z + p] + DAMPING * R_new[e * z + p];

        // np.roll(R_new, -shift): out[p] = R_new[(p+shift)%z]
        for (int p = 0; p < z; p++)
          L[col * z + p] += R_new[e * z + ((p + shift) % z)];
      }
    }

    // Syndrome check via base graph
    int fail = 0;
    for (int layer = 0; layer < bg_rows && !fail; layer++) {
      for (int p = 0; p < z && !fail; p++) {
        int syndrome = 0;
        for (int c = 0; c < bg_cols; c++) {
          int shift = B[layer * bg_cols + c];
          if (shift != -1)
            syndrome ^= (L[c * z + ((p + shift) % z)] < 0) ? 1 : 0;
        }
        if (syndrome) fail = 1;
      }
    }

    fprintf(stderr, "golden iter %d, parity=%d\n", it, fail);
    if (!fail) {
      result_iter = it + 1;
      break;
    }
  }

  for (int j = 0; j < cols; j++)
    hard_out[j] = (L[j] < 0) ? 1 : 0;

  free(L);
  free(R);
  free(edge_map);
  free(local_msgs);
  free(R_new);
  return result_iter;
}

#endif // LDPC_GOLDEN_HPP
