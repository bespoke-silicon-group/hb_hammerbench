#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#ifdef _OPENMP
#include <omp.h>
#endif

static double now_seconds(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void init_matrices(float *A, float *B, int N, int pattern)
{
  int total = N * N;
  if (pattern == 0) {
    for (int i = 0; i < total; ++i) {
      A[i] = (float)(i % 7);
      B[i] = (float)(i % 3);
    }
  } else {
    srand(12345);
    for (int i = 0; i < total; ++i) {
      A[i] = (float)rand() / RAND_MAX;
      B[i] = (float)rand() / RAND_MAX;
    }
  }
}

// Simple row-major baseline
void host_mm_simple(float *C, const float *A, const float *B, int N)
{
  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      float sum = 0.0f;
      for (int z = 0; z < N; ++z) {
        sum += A[y*N + z] * B[z*N + x];
      }
      C[y*N + x] = sum;
    }
  }
}

// Blocked/tiled version for cache locality
#define TILE 32
void host_mm_tiled(float *C, const float *A, const float *B, int N)
{
  for (int by = 0; by < N; by += TILE) {
    for (int bx = 0; bx < N; bx += TILE) {
      for (int bz = 0; bz < N; bz += TILE) {
        int y_max = (by + TILE < N) ? by + TILE : N;
        int x_max = (bx + TILE < N) ? bx + TILE : N;
        int z_max = (bz + TILE < N) ? bz + TILE : N;
        for (int y = by; y < y_max; ++y) {
          for (int x = bx; x < x_max; ++x) {
            float sum = C[y*N + x];
            for (int z = bz; z < z_max; ++z) {
              sum += A[y*N + z] * B[z*N + x];
            }
            C[y*N + x] = sum;
          }
        }
      }
    }
  }
}

// Blocked with OpenMP parallelization
void host_mm_tiled_omp(float *C, const float *A, const float *B, int N)
{
  for (int bz = 0; bz < N; bz += TILE) {
    int z_max = (bz + TILE < N) ? bz + TILE : N;
    #pragma omp parallel for collapse(2) schedule(static)
    for (int by = 0; by < N; by += TILE) {
      for (int bx = 0; bx < N; bx += TILE) {
        int y_max = (by + TILE < N) ? by + TILE : N;
        int x_max = (bx + TILE < N) ? bx + TILE : N;
        for (int y = by; y < y_max; ++y) {
          for (int x = bx; x < x_max; ++x) {
            float sum = C[y*N + x];
            for (int z = bz; z < z_max; ++z) {
              sum += A[y*N + z] * B[z*N + x];
            }
            C[y*N + x] = sum;
          }
        }
      }
    }
  }
}

float compute_sse(const float *Cref, const float *C, int N)
{
  double sse = 0.0;
  int total = N * N;
  for (int i = 0; i < total; ++i) {
    double d = (double)Cref[i] - (double)C[i];
    sse += d * d;
  }
  return (float)sse;
}

int main(int argc, char **argv)
{
  if (argc < 3) {
    printf("Usage: %s <N> <NITER> [pattern] [variant]\n", argv[0]);
    printf(" pattern: 0=deterministic, 1=random\n");
    printf(" variant: 0=simple, 1=tiled, 2=tiled+omp (default 1)\n");
    return 1;
  }
  int N = atoi(argv[1]);
  int NITER = atoi(argv[2]);
  int pattern = (argc >= 4) ? atoi(argv[3]) : 0;
  int variant = (argc >= 5) ? atoi(argv[4]) : 1;

  size_t total = (size_t)N * N;
  float *A = malloc(sizeof(float) * total);
  float *B = malloc(sizeof(float) * total);
  float *C = calloc(total, sizeof(float));
  float *Cref = malloc(sizeof(float) * total);
  if (!A || !B || !C || !Cref) {
    perror("malloc");
    return 1;
  }

  init_matrices(A, B, N, pattern);

  double t0 = now_seconds();
  for (int iter = 0; iter < NITER; ++iter) {
    memset(C, 0, total * sizeof(float));
    if (variant == 0)
      host_mm_simple(C, A, B, N);
    else if (variant == 1)
      host_mm_tiled(C, A, B, N);
    else
      host_mm_tiled_omp(C, A, B, N);
  }
  double t1 = now_seconds();

  memset(Cref, 0, total * sizeof(float));
  if (variant == 0)
    host_mm_simple(Cref, A, B, N);
  else if (variant == 1)
    host_mm_tiled(Cref, A, B, N);
  else
    host_mm_tiled_omp(Cref, A, B, N);

  float sse = compute_sse(Cref, C, N);
  double avg_time = (t1 - t0) / NITER;
  double gflops = 2.0 * N * N * N / (avg_time * 1e9);

  printf("N=%d NITER=%d variant=%d pattern=%d time=%.6f s avg=%.6f s gflops=%.2f SSE=%.6e\n",
         N, NITER, variant, pattern, t1 - t0, avg_time, gflops, sse);

  free(A); free(B); free(C); free(Cref);
  return 0;
}
