#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <immintrin.h>

#define CORES 8
#define VECTOR_WIDTH 8
#define LB 1
#define UB 801 //multiple of VECTOR_WIDTH && CORES +1

void* findSquares(void* args) {

  uint64_t inc = *(int*)args;
  uint64_t a,b,c,d,j,k,l,mc,uniq[VECTOR_WIDTH],x[9];
  __m512i __attribute__((aligned(64))) aa,bb,cc,dd,ee,ff,gg,hh,ii,xx;
  __m512d __attribute__((aligned(64))) rootf,rooti;
  __mmask16 __attribute__((aligned(32))) mask;
  union __attribute__((aligned(64))) squareVecPtr {__m512i *v; uint64_t *s;} ptr;
  uint64_t *squares = malloc((UB - LB) * sizeof(uint64_t));

  for(uint64_t s = LB; s < UB; s++)
    squares[s - LB] = s*s;

  for(a = LB; a < UB; a++) {
    aa = _mm512_set1_epi64(squares[a-LB]);
    for(b = LB; b < UB; b++) {
      bb = _mm512_set1_epi64(squares[b-LB]);
      for(c = LB+inc; c < UB; c+=CORES) {
        cc = _mm512_set1_epi64(squares[c-LB]); //row 1
        mc = squares[a-LB] + squares[b-LB] + squares[c-LB]; //magic constant
        for(d = LB; d < UB - VECTOR_WIDTH; d+=VECTOR_WIDTH) {
          dd = _mm512_loadu_epi64(&squares[d-LB]); //vector sequence
          //calc gg
          gg = _mm512_set1_epi64(squares[b-LB] + squares[c-LB]);
          gg = _mm512_sub_epi64(gg, dd); //column 1
          //calc ee
          ee = _mm512_set1_epi64(mc - squares[c-LB]);
          ee = _mm512_sub_epi64(ee, gg); //diag 1
          //check for negatives
          mask = _mm512_movepi64_mask(gg);
          mask |= _mm512_movepi64_mask(ee);
          if(mask == 65535) break;

          //check gg square
          rootf = _mm512_cvtepi64_pd(gg); //int -> floats
          rooti = _mm512_sqrt_pd(rootf); //square root
          rootf = _mm512_floor_pd(rooti); //floor
          mask |= _mm512_cmp_pd_mask(rootf, rooti, _CMP_NEQ_OQ); //Cumulative T/F bitmask
          if(mask == 65535) continue;
          //check ee square
          rootf = _mm512_cvtepi64_pd(ee);
          rooti = _mm512_sqrt_pd(rootf);
          rootf = _mm512_floor_pd(rooti);
          mask |= _mm512_cmp_pd_mask(rootf, rooti, _CMP_NEQ_OQ);
          if(mask == 65535) continue;

          //calc hh
          hh = _mm512_set1_epi64(squares[a-LB] + squares[c-LB]);
          hh = _mm512_sub_epi64(hh, ee); //column 2
          //check for negatives
          mask |= _mm512_movepi64_mask(hh);
          if(mask == 65535) break;
          //calc ff
          ff = _mm512_set1_epi64(mc);
          ff = _mm512_sub_epi64(ff, dd);
          ff = _mm512_sub_epi64(ff, ee); //row 2
          //check for negatives
          mask |= _mm512_movepi64_mask(ff);
          if(mask == 65535) break;
          //calc ii
          ii = _mm512_set1_epi64(mc);
          ii = _mm512_sub_epi64(ii, gg);
          ii = _mm512_sub_epi64(ii, hh); //row 3
          //check for negatives
          mask |= _mm512_movepi64_mask(ii);
          if(mask == 65535) break;

          //diag 2 == mc
          xx = _mm512_add_epi64(bb, cc);
          xx = _mm512_sub_epi64(xx, ee);
          mask |= ~_mm512_cmpeq_epi64_mask(xx, ii);
          if(mask == 65535) continue;
          //column 3 == mc
          xx = _mm512_add_epi64(aa, bb);
          xx = _mm512_sub_epi64(xx, ff);
          mask |= ~_mm512_cmpeq_epi64_mask(xx, ii);
          if(mask == 65535) continue;
          //check hh square
          rootf = _mm512_cvtepi64_pd(hh);
          rooti = _mm512_sqrt_pd(rootf);
          rootf = _mm512_floor_pd(rooti);
          mask |= _mm512_cmp_pd_mask(rootf, rooti, _CMP_NEQ_OQ);
          if(mask == 65535) continue;
          //check ff square
          rootf = _mm512_cvtepi64_pd(ff);
          rooti = _mm512_sqrt_pd(rootf);
          rootf = _mm512_floor_pd(rooti);
          mask |= _mm512_cmp_pd_mask(rootf, rooti, _CMP_NEQ_OQ);
          if(mask == 65535) continue;
          //check ii square
          rootf = _mm512_cvtepi64_pd(ii);
          rooti = _mm512_sqrt_pd(rootf);
          rootf = _mm512_floor_pd(rooti);
          mask |= _mm512_cmp_pd_mask(rootf, rooti, _CMP_NEQ_OQ);
          if(mask == 65535) continue;

          //if registers hold multiple solutions. unlikely so just notify...
          if(__builtin_popcount(mask) < VECTOR_WIDTH - 1) printf("POPCNT %i\n", VECTOR_WIDTH - __builtin_popcount(mask));
          //check for uniqueness
          mask = __builtin_ctz(~mask);
          ptr.s = uniq;
          _mm512_storeu_epi64(ptr.v,aa); x[0] = ptr.s[mask];
          _mm512_storeu_epi64(ptr.v,bb); x[1] = ptr.s[mask];
          _mm512_storeu_epi64(ptr.v,cc); x[2] = ptr.s[mask];
          _mm512_storeu_epi64(ptr.v,dd); x[3] = ptr.s[mask];
          _mm512_storeu_epi64(ptr.v,ee); x[4] = ptr.s[mask];
          _mm512_storeu_epi64(ptr.v,ff); x[5] = ptr.s[mask];
          _mm512_storeu_epi64(ptr.v,gg); x[6] = ptr.s[mask];
          _mm512_storeu_epi64(ptr.v,hh); x[7] = ptr.s[mask];
          _mm512_storeu_epi64(ptr.v,ii); x[8] = ptr.s[mask];

          l = 0;
          for(j = 0; j < 8; j++)
            for(k = j+1; k < 9; k++)
              l += x[j] == x[k];
          if(l < 3) printf("Magic Square! %lu %lu %lu %lu %lu %lu %lu %lu %lu\tUniqueness %li\n",x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8],9-l);
        }
      }
    }
  }
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

