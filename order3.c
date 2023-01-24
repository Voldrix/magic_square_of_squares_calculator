#include <stdio.h>
#include <pthread.h>
#include <math.h>

#define CORES 8
#define LB 1
#define UB 801 //multiple of CORES +1

inline int square(int ss) {
  switch((ss & 0xF)) {
    case 0: case 1: case 4: case 9:
      int s = sqrt(ss);
      return s*s == ss;
    default:
      return 0;
  }
}

void* findSquares(void* args) {

  int inc = *(int*)args;
  int __attribute__((aligned(32))) a,b,c,d,j,k,l,aa,bb,cc,dd,ee,ff,gg,hh,ii,mc,squares[UB];
  int* x[9] = {&aa,&bb,&cc,&dd,&ee,&ff,&gg,&hh,&ii};

  for(int s = LB; s < UB; s++)
    squares[s] = s*s;

  for(a = LB; a < UB; a++) {
    for(b = LB; b < UB; b++) {
      for(c = LB+inc; c < UB; c+=CORES) {
        mc = squares[a] + squares[b] + squares[c]; //row 1
        for(d = LB; d < UB; d++) {
          gg = squares[b] + squares[c] - squares[d]; //column 1
          ee = mc - squares[c] - gg; //diag 1
          if(gg < 1 || ee < 1 ) break;
          ff = mc - squares[d] - ee; //row 2
          if(ff < 1) break;
          hh = squares[a] + squares[c] - ee; //colum 2
          ii = mc - gg - hh; //row 3
          if(hh < 1 || ii < 1 || squares[b] + squares[c] != ee + ii || squares[a] + squares[b] != ff + ii) continue; //diag 2, column 3
          if(!square(gg) || !square(ee) || !square(ff) || !square(hh) || !square(ii)) continue;

          //check for uniqueness
          aa=squares[a]; bb=squares[b]; cc=squares[c]; dd=squares[d];
          l = 0;
          for(j = 0; j < 8; j++)
            for(k = j+1; k < 9; k++)
              l += *x[j] == *x[k];
          if(l < 3) printf("Magic Square! %i %i %i %i %i %i %i %i %i\tUniqueness %i\n",aa,bb,cc,dd,ee,ff,gg,hh,ii,9-l);
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

