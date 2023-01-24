CC = gcc
CFLAGS = -Ofast -Wall -ffast-math -fno-math-errno -lpthread
LDFLAGS = 
PROC := $(shell grep -om1 Intel /proc/cpuinfo)
ifeq ($(PROC),Intel)
  TUNE = -mtune=intel
else
  TUNE = -mtune=native
endif

.PHONY: all clean
.SILENT: avx2 avx512 order3 fma_detect clean

all: clean avx2 order3

avx2: avx2.c
	$(CC) $(CFLAGS) -mavx2 $(TUNE) $^ $(LDFLAGS) -o avx2
	echo "avx2"

avx512: avx512.c
	$(CC) $(CFLAGS) -mavx512dq $(TUNE) $^ $(LDFLAGS) -o avx512
	echo "avx512"

order3:
	echo "order3"

fma: fma_detect.c
	$(CC) -mavx512f $^ -o fma_detect
	echo "fma_detect"

clean:
	rm -f avx2 avx512 order3 fma_detect *.o

