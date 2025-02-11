#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>

#define CORES 8
#define LB 1
#define UB 8193 //multiple of CORES +1

static inline int square(uint64_t ss) {
  switch((ss & 0xF)) {
    case 0: case 1: case 4: case 9:
      uint64_t s = sqrt(ss);
      return s*s == ss;
    default:
      return 0;
  }
}

void* findSquares(void* args) {

  uint64_t inc = *(int*)args;
  uint64_t __attribute__((aligned(64))) a,b,c,d,j,k,l,aa,bb,cc,dd,ee,ff,gg,hh,ii,mc;
  uint64_t *x[9] = {&aa,&bb,&cc,&dd,&ee,&ff,&gg,&hh,&ii};
  uint64_t *squares = malloc((UB - LB) * sizeof(uint64_t));

  for(uint64_t s = LB; s < UB; s++)
    squares[s - LB] = s*s;

  for(a = LB; a < UB; a++) {
    for(b = LB; b < UB; b++) {
      for(c = LB+inc; c < UB; c+=CORES) {
        mc = squares[a-LB] + squares[b-LB] + squares[c-LB]; //row 1
        for(d = LB; d < UB; d++) {
          gg = squares[b-LB] + squares[c-LB] - squares[d-LB]; //column 1
          ee = mc - squares[c-LB] - gg; //diag 1
          if(gg < 1 || ee < 1 ) break;
          ff = mc - squares[d-LB] - ee; //row 2
          if(ff < 1) break;
          hh = squares[a-LB] + squares[c-LB] - ee; //colum 2
          ii = mc - gg - hh; //row 3
          if(squares[b-LB] + squares[c-LB] != ee + ii || squares[a-LB] + squares[b-LB] != ff + ii || hh < 1 || ii < 1) continue; //diag 2, column 3
          if(!square(gg) || !square(ee) || !square(ff) || !square(hh) || !square(ii)) continue;

          //check for uniqueness
          aa=squares[a-LB]; bb=squares[b-LB]; cc=squares[c-LB]; dd=squares[d-LB];
          l = 0;
          for(j = 0; j < 8; j++)
            for(k = j+1; k < 9; k++)
              l += *x[j] == *x[k];
          if(l < 3) printf("Magic Square! %lu %lu %lu %lu %lu %lu %lu %lu %lu\tUniqueness %li\n",aa,bb,cc,dd,ee,ff,gg,hh,ii,9-l);
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

