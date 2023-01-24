#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>

static uint64_t rdtsc(void) {
  unsigned int ax, dx;
  __asm__ __volatile__ ("rdtsc" : "=a"(ax), "=d"(dx));
  return ((((uint64_t)dx) << 32) | ax);
}

void fma_shuffle_tpt() {
  __m512d __attribute__((aligned(64))) vec = {1, 1, 1, 1, 1, 1, 1, 1};
  __m512i __attribute__((aligned(64))) shuf = {0, 1, 2, 3, 4, 5, 6, 7};
  for(int i = 0; i < 2048; i++) {
    vec = _mm512_fmadd_pd(vec, vec, vec);
    shuf = _mm512_permutexvar_epi32(shuf, shuf);
  }
}

void fma_only_tpt() {
  __m512d __attribute__((aligned(64))) vec = {1, 1, 1, 1, 1, 1, 1, 1};
  for(int i = 0; i < 2048; i++) {
    vec = _mm512_fmadd_pd(vec, vec, vec);
  }
}

int main() {

  if(!__builtin_cpu_supports("avx512f")) {
    printf("This CPU does not support AVX-512\n");
    return 1;
  }

  int start = rdtsc();
  fma_shuffle_tpt();
  int shuf_test = rdtsc() - start;

  start = rdtsc();
  fma_only_tpt();
  int fma_test = rdtsc() - start;

  //should be close to 2.0 for 2 FMAs
  int fma_units = ((shuf_test / fma_test) > 1.5) ? 2 : 1;

  printf("\nShuffle + FMA time: %i\nOnly FMA time: %i\n\n", shuf_test, fma_test);
  printf("Your CPU probably has _%i_ AVX-512 FMA unit(s) per core\n\n", fma_units);
  return 0;
}
