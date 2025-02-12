#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <immintrin.h>

#define CORES 8
#define VECTOR_WIDTH 4
#define LB 1
#define UB 2401 //multiple of VECTOR_WIDTH && CORES +1

void* findSquares(void* args) {

  uint64_t inc = *(int*)args;
  uint64_t a,b,c,d,j,k,l,mc,uniq[VECTOR_WIDTH],x[9];
  int mask;
  __m256i __attribute__((aligned(64))) aa,bb,cc,dd,ee,ff,gg,hh,ii,xx;
  __m256d __attribute__((aligned(64))) rootf,rooti;
  union __attribute__((aligned(64))) squareVecPtr {__m256i *v; uint64_t *s;} ptr;
  uint64_t *squares = malloc((UB - LB) * sizeof(uint64_t));
  _mm256_zeroall();

  for(uint64_t s = LB; s < UB; s++)
    squares[s - LB] = s*s;

  for(a = LB; a < UB; a++) {
    aa = _mm256_set1_epi64x(squares[a-LB]);
    for(b = LB; b < UB; b++) {
      bb = _mm256_set1_epi64x(squares[b-LB]);
      for(c = LB+inc; c < UB; c+=CORES) {
        cc = _mm256_set1_epi64x(squares[c-LB]); //row 1
        mc = squares[a-LB] + squares[b-LB] + squares[c-LB]; //magic constant
        for(d = LB; d < UB - VECTOR_WIDTH; d+=VECTOR_WIDTH) {
          dd = _mm256_loadu_epi64(&squares[d-LB]); //vector sequence
          //calc gg
          gg = _mm256_set1_epi64x(squares[b-LB] + squares[c-LB]);
          gg = _mm256_sub_epi64(gg, dd); //column 1
          //calc ee
          ee = _mm256_add_epi64(aa, bb);
          ee = _mm256_sub_epi64(ee, gg); //diag 1
          //check for negatives
          mask = _mm256_movemask_pd(_mm256_castsi256_pd(gg));
          mask |= _mm256_movemask_pd(_mm256_castsi256_pd(ee));
          if(mask == 15) break;

          //check gg square
          rootf = _mm256_cvtepi64_pd(gg); //int -> floats
          rooti = _mm256_sqrt_pd(rootf); //square root
          rootf = _mm256_floor_pd(rooti); //floor
          rooti = _mm256_cmp_pd(rootf, rooti, _CMP_NEQ_OQ); //Cumulative T/F bitmask
          mask |= _mm256_movemask_pd(rooti);
          if(mask == 15) continue;
          //check ee square
          rootf = _mm256_cvtepi64_pd(ee);
          rooti = _mm256_sqrt_pd(rootf);
          rootf = _mm256_floor_pd(rooti);
          rooti = _mm256_cmp_pd(rootf, rooti, _CMP_NEQ_OQ);
          mask |= _mm256_movemask_pd(rooti);
          if(mask == 15) continue;

          //calc hh
          hh = _mm256_set1_epi64x(squares[a-LB] + squares[c-LB]);
          hh = _mm256_sub_epi64(hh, ee); //column 2
          //check for negatives
          mask |= _mm256_movemask_pd(_mm256_castsi256_pd(hh));
          if(mask == 15) break;
          //calc ff
          ff = _mm256_set1_epi64x(mc);
          ff = _mm256_sub_epi64(ff, dd);
          ff = _mm256_sub_epi64(ff, ee); //row 2
          //check for negatives
          mask |= _mm256_movemask_pd(_mm256_castsi256_pd(ff));
          //if(mask == 15) break;
          //calc ii
          ii = _mm256_set1_epi64x(mc);
          ii = _mm256_sub_epi64(ii, gg);
          ii = _mm256_sub_epi64(ii, hh); //row 3
          //check for negatives
          mask |= _mm256_movemask_pd(_mm256_castsi256_pd(ii));
          if(mask == 15) break;

          //diag 2 == mc
          xx = _mm256_add_epi64(bb, cc);
          xx = _mm256_sub_epi64(xx, ee);
          xx = _mm256_cmpeq_epi64(xx, ii);
          mask |= ~_mm256_movemask_pd(_mm256_castsi256_pd(xx));
          if(mask == 15) continue;
          //column 3 == mc
          xx = _mm256_add_epi64(aa, bb);
          xx = _mm256_sub_epi64(xx, ff);
          xx = _mm256_cmpeq_epi64(xx, ii);
          mask |= ~_mm256_movemask_pd(_mm256_castsi256_pd(xx));
          if(mask == 15) continue;
          //check hh square
          rootf = _mm256_cvtepi64_pd(hh);
          rooti = _mm256_sqrt_pd(rootf);
          rootf = _mm256_floor_pd(rooti);
          rooti = _mm256_cmp_pd(rootf, rooti, _CMP_NEQ_OQ);
          mask |= _mm256_movemask_pd(rooti);
          if(mask == 15) continue;
          //check ff square
          rootf = _mm256_cvtepi64_pd(ff);
          rooti = _mm256_sqrt_pd(rootf);
          rootf = _mm256_floor_pd(rooti);
          rooti = _mm256_cmp_pd(rootf, rooti, _CMP_NEQ_OQ);
          mask |= _mm256_movemask_pd(rooti);
          if(mask == 15) continue;
          //check ii square
          rootf = _mm256_cvtepi64_pd(ii);
          rooti = _mm256_sqrt_pd(rootf);
          rootf = _mm256_floor_pd(rooti);
          rooti = _mm256_cmp_pd(rootf, rooti, _CMP_NEQ_OQ);
          mask |= _mm256_movemask_pd(rooti);
          if(mask == 15) continue;

          //if registers hold multiple solutions. unlikely so just notify...
          if(__builtin_popcount(mask) < VECTOR_WIDTH - 1) printf("POPCNT %i\n", VECTOR_WIDTH - __builtin_popcount(mask));
          //check for uniqueness
          mask = __builtin_ctz(~mask);
          ptr.s = uniq;
          _mm256_storeu_epi64(ptr.v,aa); x[0] = ptr.s[mask];
          _mm256_storeu_epi64(ptr.v,bb); x[1] = ptr.s[mask];
          _mm256_storeu_epi64(ptr.v,cc); x[2] = ptr.s[mask];
          _mm256_storeu_epi64(ptr.v,dd); x[3] = ptr.s[mask];
          _mm256_storeu_epi64(ptr.v,ee); x[4] = ptr.s[mask];
          _mm256_storeu_epi64(ptr.v,ff); x[5] = ptr.s[mask];
          _mm256_storeu_epi64(ptr.v,gg); x[6] = ptr.s[mask];
          _mm256_storeu_epi64(ptr.v,hh); x[7] = ptr.s[mask];
          _mm256_storeu_epi64(ptr.v,ii); x[8] = ptr.s[mask];

          l = 0;
          for(j = 0; j < 8; j++)
            for(k = j+1; k < 9; k++)
              l += x[j] == x[k];
          if(l < 3) printf("Magic Square! %lu %lu %lu %lu %lu %lu %lu %lu %lu\tUniqueness %li\n",x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8],9-l);
        }
      }
    }
  }
  free(squares);
  return 0;
}

int main() {
  int threadNum[CORES];
  pthread_t thread[CORES];
  for(int coreOffset = 0; coreOffset < CORES; coreOffset++) {
    threadNum[coreOffset] = coreOffset;
    pthread_create(&thread[coreOffset], NULL, &findSquares, &threadNum[coreOffset]);
  }
  for(int coreOffset = 0; coreOffset < CORES; coreOffset++)
    pthread_join(thread[coreOffset], NULL);
  return 0;
}

