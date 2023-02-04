#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <immintrin.h>

#define CORES 8
#define VECTOR_WIDTH 8
#define LB 1
#define UB 801 //multiple of VECTOR_WIDTH && CORES +1

void* findSquares(void* args) {

  int __attribute__((aligned(32))) inc = *(int*)args;
  int __attribute__((aligned(32))) a,b,c,d,j,k,l,mc,uniq[VECTOR_WIDTH],squares[UB],x[9];
  __m256i __attribute__((aligned(32))) aa,bb,cc,dd,ee,ff,gg,hh,ii,xx;
  __m256 __attribute__((aligned(32))) rootf,rooti;
  __mmask8 __attribute__((aligned(32))) mask;
  union __attribute__((aligned(32))) squareVecPtr {__m256i* v; int* s;} ptr;
  _mm256_zeroall();

  for(int s = LB; s < UB; s++)
    squares[s] = s*s;

  for(a = LB; a < UB; a++) {
    aa = _mm256_set1_epi32(squares[a]);
    for(b = LB; b < UB; b++) {
      bb = _mm256_set1_epi32(squares[b]);
      for(c = LB+inc; c < UB; c+=CORES) {
        cc = _mm256_set1_epi32(squares[c]); //row 1
        mc = squares[a] + squares[b] + squares[c]; //magic constant
        for(d = LB; d < UB - VECTOR_WIDTH; d+=VECTOR_WIDTH) {
          ptr.s = &squares[d];
          dd = *ptr.v; //vector sequence
          //calc gg
          gg = _mm256_set1_epi32(squares[b] + squares[c]);
          gg = _mm256_sub_epi32(gg, dd); //column 1
          //calc ee
          ee = _mm256_add_epi32(aa, bb);
          ee = _mm256_sub_epi32(ee, gg); //diag 1
          //check for negatives
          mask = _mm256_movemask_ps(_mm256_castsi256_ps(gg));
          mask |= _mm256_movemask_ps(_mm256_castsi256_ps(ee));
          if(mask == 255) break;

          //check gg square
          rootf = _mm256_cvtepi32_ps(gg); //int -> floats
          rooti = _mm256_sqrt_ps(rootf); //square root
          rootf = _mm256_floor_ps(rooti); //floor
          rooti = _mm256_cmp_ps(rootf, rooti, _CMP_NEQ_OQ); //Cumulative T/F bitmask
          mask |= _mm256_movemask_ps(rooti);
          if(mask == 255) continue;
          //check ee square
          rootf = _mm256_cvtepi32_ps(ee);
          rooti = _mm256_sqrt_ps(rootf);
          rootf = _mm256_floor_ps(rooti);
          rooti = _mm256_cmp_ps(rootf, rooti, _CMP_NEQ_OQ);
          mask |= _mm256_movemask_ps(rooti);
          if(mask == 255) continue;

          //calc hh
          hh = _mm256_set1_epi32(squares[a] + squares[c]);
          hh = _mm256_sub_epi32(hh, ee); //column 2
          //check for negatives
          mask |= _mm256_movemask_ps(_mm256_castsi256_ps(hh));
          if(mask == 255) break;
          //calc ff
          ff = _mm256_set1_epi32(mc);
          ff = _mm256_sub_epi32(ff, dd);
          ff = _mm256_sub_epi32(ff, ee); //row 2
          //check for negatives
          mask |= _mm256_movemask_ps(_mm256_castsi256_ps(ff));
          //if(mask == 255) break;
          //calc ii
          ii = _mm256_set1_epi32(mc);
          ii = _mm256_sub_epi32(ii, gg);
          ii = _mm256_sub_epi32(ii, hh); //row 3
          //check for negatives
          mask |= _mm256_movemask_ps(_mm256_castsi256_ps(ii));
          if(mask == 255) break;

          //diag 2 == mc
          xx = _mm256_add_epi32(bb, cc);
          xx = _mm256_sub_epi32(xx, ee);
          xx = _mm256_cmpeq_epi32(xx, ii);
          mask |= ~_mm256_movemask_ps(_mm256_castsi256_ps(xx));
          if(mask == 255) continue;
          //column 3 == mc
          xx = _mm256_add_epi32(aa, bb);
          xx = _mm256_sub_epi32(xx, ff);
          xx = _mm256_cmpeq_epi32(xx, ii);
          mask |= ~_mm256_movemask_ps(_mm256_castsi256_ps(xx));
          if(mask == 255) continue;
          //check hh square
          rootf = _mm256_cvtepi32_ps(hh);
          rooti = _mm256_sqrt_ps(rootf);
          rootf = _mm256_floor_ps(rooti);
          rooti = _mm256_cmp_ps(rootf, rooti, _CMP_NEQ_OQ);
          mask |= _mm256_movemask_ps(rooti);
          if(mask == 255) continue;
          //check ff square
          rootf = _mm256_cvtepi32_ps(ff);
          rooti = _mm256_sqrt_ps(rootf);
          rootf = _mm256_floor_ps(rooti);
          rooti = _mm256_cmp_ps(rootf, rooti, _CMP_NEQ_OQ);
          mask |= _mm256_movemask_ps(rooti);
          if(mask == 255) continue;
          //check ii square
          rootf = _mm256_cvtepi32_ps(ii);
          rooti = _mm256_sqrt_ps(rootf);
          rootf = _mm256_floor_ps(rooti);
          rooti = _mm256_cmp_ps(rootf, rooti, _CMP_NEQ_OQ);
          mask |= _mm256_movemask_ps(rooti);
          if(mask == 255) continue;

          //if registers hold multiple solutions. unlikely so just notify...
          if(__builtin_popcount(mask) < 7) printf("POPCNT %i\n",8 - __builtin_popcount(mask));
          //check for uniqueness
          mask = __builtin_ctz(~mask);
          ptr.s = uniq;
          _mm256_store_si256(ptr.v,aa); x[0] = ptr.s[mask];
          _mm256_store_si256(ptr.v,bb); x[1] = ptr.s[mask];
          _mm256_store_si256(ptr.v,cc); x[2] = ptr.s[mask];
          _mm256_store_si256(ptr.v,dd); x[3] = ptr.s[mask];
          _mm256_store_si256(ptr.v,ee); x[4] = ptr.s[mask];
          _mm256_store_si256(ptr.v,ff); x[5] = ptr.s[mask];
          _mm256_store_si256(ptr.v,gg); x[6] = ptr.s[mask];
          _mm256_store_si256(ptr.v,hh); x[7] = ptr.s[mask];
          _mm256_store_si256(ptr.v,ii); x[8] = ptr.s[mask];

          l = 0;
          for(j = 0; j < 8; j++)
            for(k = j+1; k < 9; k++)
              l += x[j] == x[k];
          if(l < 3) printf("Magic Square! %i %i %i %i %i %i %i %i %i\tUniqueness %i\n",x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8],9-l);
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

