#include <cello/host/cello.hpp>
#include "common256.hpp"

template <typename T>
using hb_pointer = bsg_global_pointer::pointer<T>;

#define If (std::complex<float>(0.0,1.0))

#define PRECISION (15)
int is_close(FP32Complex n, FP32Complex r) {
  float nr = n.real(), ni = n.imag();
  float rr = r.real(), ri = r.imag();
  float dr = nr-rr, di = ni-ri;
  float dist = sqrt(dr*dr+di*di);
  if (dist < PRECISION)
    return 1;
  else
    return 0;
}

int verify_fft (matrix & out_matrix) {
  int N = NUM_POINTS*NUM_POINTS;
  int r  = N / 16;
  int ar = N - r;
  FP32Complex *out = &out_matrix[0][0];
  for (int i = 0; i < N; i++) {
    float rr = out[i].real(), ii = out[i].imag();
    //printf("%d-th result is %.6f+%.6fi (0x%08X 0x%08X)\n", i, rr, ii, *(uint32_t*)&rr, *(uint32_t*)&ii);
  }

  bool fail = false;
  for (int i = 0; i < N; i++) {
      std::complex<float> ref = 0.0;
    //printf("%d-th component is %.3f+%.3fi\n", i, out[i].real(), out[i].imag());
    if ((i == r) || (i == ar)) {
      ref = N / 2.0;
    } else {
      ref = 0.0;
    }

    if (!is_close(out[i], ref)) {
      printf("Mismatch: out[%d]: %.3f+%.3fi; ref is %.3f+%.3fi\n"
             , i
             , out[i].real()
             , out[i].imag()
             , ref.real()
             , ref.imag()
             );
      fail = true;
    }
  }
  return (fail ? 1 : 0);
}

class program : public cello::program {
    int init(int argc, char *argv[]) {
        BSG_CUDA_CALL(cello::program::init(argc, argv));
        //printf("sizeof(matrix) = %zu\n", sizeof(matrix));
        d_in = new batch_vector::mirror(find<batch_vector>("in"));
        d_out = new batch_vector::mirror(find<batch_vector>("out"));
        d_twiddle = find<matrix>("twiddle");
        num_iters = atoi(argv[2]);
        printf("%d point FFT: %d iters\n", NUM_POINTS, num_iters);
        return 0;
    }

    int input() {
        BSG_CUDA_CALL(cello::program::input());
        for (int r = 0; r < NUM_POINTS; r++) {
            for (int c = 0; c < NUM_POINTS; c++) {
                // input
                int j = r*NUM_POINTS + c;
                A[r][c] = cosf(j*M_PI/8.0);
                // output
                float ref_sinf = sinf(-2.0f * M_PI * static_cast<float>(r*c)/static_cast<float>(NUM_POINTS*NUM_POINTS));
                float ref_cosf = cosf(-2.0f * M_PI * static_cast<float>(r*c)/static_cast<float>(NUM_POINTS*NUM_POINTS));
                h_twiddle[r][c] = ref_cosf + If * ref_sinf;
            }
        }

        h_in.resize(num_iters);
        h_out.resize(num_iters);

        for (int i = 0; i < num_iters; i++) {
            h_in[i] = A;
        }
        BSG_CUDA_CALL(d_in->init_host_from(h_in));
        BSG_CUDA_CALL(d_out->init_host_from(h_out));
        // initialize twiddle
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(&mc, pod_id) {
            jobs_in[pod_id].push_back({d_twiddle.to_local(), &h_twiddle, sizeof(h_twiddle)});
        }
        // sync device
        BSG_CUDA_CALL(d_in->sync_device(jobs_in));
        BSG_CUDA_CALL(d_out->sync_device(jobs_in));
        return 0;
    }

    int output() {
        BSG_CUDA_CALL(cello::program::output());
        BSG_CUDA_CALL(d_out->sync_host(jobs_out));
        return 0;
    }

    int check_output() {
        BSG_CUDA_CALL(cello::program::check_output());
        bool fail = false;
        for (int i = 0; i < num_iters; i++) {
            if (verify_fft(d_out->at(i)))  {
                fail = true;
            }
        }

        if (fail) {
            return HB_MC_FAIL;
        } else {
            return HB_MC_SUCCESS;
        }
    }
    batch_vector::mirror *d_in;
    batch_vector::mirror *d_out;
    std::vector<matrix>   h_in;
    std::vector<matrix>   h_out;
    hb_pointer<matrix> d_twiddle;
    matrix  h_twiddle;
    matrix  A, B;
    int num_iters = 0;
};

cello::program *make_program() {
    return new program();
}
