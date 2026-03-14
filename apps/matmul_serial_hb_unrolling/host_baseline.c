#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

static double now_seconds(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void init_matrices(float *A, float *B, int N, int pattern)
{
  int total = N * N;
  if (pattern == 0) { // deterministic small values
    for (int i = 0; i < total; ++i) {
      A[i] = (float)(i % 7);
      B[i] = (float)(i % 3);
    }
  } else { // random
    srand(12345);
    for (int i = 0; i < total; ++i) {
      A[i] = (float)rand() / RAND_MAX;
      B[i] = (float)rand() / RAND_MAX;
    }
  }
}

void host_mm(float *C, const float *A, const float *B, int N)
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
    printf("Usage: %s <N> <NITER> [pattern]\n", argv[0]);
    printf(" pattern=0 deterministic, 1 random (default 0)\n");
    return 1;
  }
  int N = atoi(argv[1]);
  int NITER = atoi(argv[2]);
  int pattern = (argc >= 4) ? atoi(argv[3]) : 0;

  size_t total = (size_t)N * N;
  float *A = malloc(sizeof(float) * total * NITER);
  float *B = malloc(sizeof(float) * total * NITER);
  float *C = malloc(sizeof(float) * total * NITER);
  float *Cref = malloc(sizeof(float) * total);
  if (!A || !B || !C || !Cref) {
    perror("malloc");
    return 1;
  }

  // Initialize
  for (int iter = 0; iter < NITER; ++iter) {
    init_matrices(&A[iter*total], &B[iter*total], N, pattern);
  }

  double t0 = now_seconds();
  for (int iter = 0; iter < NITER; ++iter) {
    host_mm(&C[iter*total], &A[iter*total], &B[iter*total], N);
  }
  double t1 = now_seconds();

  // Compute reference on first iteration for correctness
  host_mm(Cref, &A[0], &B[0], N);
  float sse = compute_sse(Cref, &C[0], N);

  printf("N=%d NITER=%d time=%.6f s avg=%.6f s SSE=%.6e\n",
         N, NITER, t1 - t0, (t1 - t0) / NITER, sse);

  free(A); free(B); free(C); free(Cref);
  return 0;
}
