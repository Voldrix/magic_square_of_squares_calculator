#include <stdio.h>
#include <pthread.h>

#define CORES 8
#define LB 1
#define UB 81 //multiple of cores +1

void* findSquares(void* args) {

  int coreOffset = *(int*)args;
  int __attribute__((aligned(32))) mc,a1,a2,a3,a4,a5,a6,a7,a8,a9,b1,b2,b3,b4,b5,b6,b7,b8,b9,c1,c2,c3,c4,c5,c6,c7,c8,c9,uba4,uba5,j,k,l;
  int* x[27] = {&a1,&a2,&a3,&a4,&a5,&a6,&a7,&a8,&a9,&b1,&b2,&b3,&b4,&b5,&b6,&b7,&b8,&b9,&c1,&c2,&c3,&c4,&c5,&c6,&c7,&c8,&c9};

  for(a1 = LB+coreOffset; a1 < UB; a1+=CORES) {
    for(a2 = LB; a2 < UB; a2++) {
      for(a3 = LB; a3 < UB; a3++) {
        mc = a1 + a2 + a3; //row A1
        uba4 = (mc < UB) * mc + (mc >= UB) * UB; //upper bound for a4
        for(a4 = LB; a4 < uba4; a4++) {
          uba5 = (mc < UB) * mc + (mc >= UB) * UB; //upper bound for a5
          for(a5 = LB; a5 < uba5; a5++) {
            a6 = mc - a4 - a5; //row A2, can be negative
            a7 = mc - a1 - a4; //column A1, can't be negative
            a8 = mc - a2 - a5; //column A2
            a9 = mc - a7 - a8; //row A3
            if(a1 + a2 != a6 + a9 || a6 < 1 || a8 < 1 || a9 < 1) continue; //column A3
            for(b5 = LB; b5 < mc - a5; b5++) {
              c5 = mc - a5 - b5; //piller 5, can't be negative
              c1 = mc - a9 - b5; //diag 4
              c3 = mc - a7 - b5; //diag 3
              c7 = mc - a3 - b5; //diag 2
              c9 = mc - a1 - b5; //diag 1
              if(c1 < 1 || c3 < 1 || c7 < 1 || c9 < 1) continue;
              c4 = mc - c7 - c1; //column C1
              c2 = mc - c1 - c3; //row C1
              c6 = mc - c3 - c9; //column C3
              c8 = mc - c9 - c7; //row C3
              if(c2 < 1 || c4 < 1 || c6 < 1 || c8 < 1) continue;
              if(mc - c5 != c2 + c8 || mc - c5 != c4 + c6) continue; //row C2, column C2
              b1 = mc - a1 - c1; //pillar 1
              b2 = mc - a2 - c2; //pillar 2
              b3 = mc - a3 - c3; //pillar 3
              b4 = mc - a4 - c4; //pillar 4
              b6 = mc - a6 - c6; //pillar 6
              b7 = mc - a7 - c7; //pillar 7
              b8 = mc - a8 - c8; //pillar 8
              b9 = mc - a9 - c9; //pillar 9
              if(b3 < 1 || b4 < 1 || b8 < 1 || c9 < 1) continue;
              if(mc - b1 != b4 + b7 || mc - b1 != b2 + b3) continue; //row B1, column B1
              if(mc - b5 != b4 + b6 || mc - b5 != b2 + b8) continue; //row B2, column B2
              if(mc - b9 != b7 + b8 || mc - b9 != b3 + b6) continue; //row B3, column B3

              //check for uniqueness
              l = 0;
              for(j = 0; j < 26; j++)
                for(k = j+1; k < 27; k++)
                  l += *x[j] == *x[k];
              if(l == 0) printf("  %i %i %i\t\t%i %i %i\t\t%i %i %i\n  %i %i %i\t\t%i %i %i\t\t%i %i %i\n  %i %i %i\t\t%i %i %i\t\t%i %i %i\nMagic Constant %i\n\n",a1,a2,a3,b1,b2,b3,c1,c2,c3,a4,a5,a6,b4,b5,b6,c4,c5,c6,a7,a8,a9,b7,b8,b9,c7,c8,c9,mc);
            }
          }
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

