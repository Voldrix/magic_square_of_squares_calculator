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
.SILENT: avx2 avx512 scaler cube fma_detect clean

all: clean avx2 scaler

avx2: avx2.c avx2_64.c
	$(CC) $(CFLAGS) -mavx2 $(TUNE) avx2.c $(LDFLAGS) -o avx2
	$(CC) $(CFLAGS) -mavx2 -mavx512dq -mavx512vl $(TUNE) avx2_64.c $(LDFLAGS) -o avx2_64
	echo "avx2 & avx2_64"

avx512: avx512.c
	$(CC) $(CFLAGS) -mavx512dq $(TUNE) $^ $(LDFLAGS) -o avx512
	echo "avx512"

cube: cube.c
	$(CC) $(CFLAGS) $(TUNE) $^ $(LDFLAGS) -o cube
	echo "cube"

scaler: scaler.c scaler_64.c
	$(CC) $(CFLAGS) $(TUNE) scaler.c $(LDFLAGS) -o scaler
	$(CC) $(CFLAGS) -mavx2 $(TUNE) scaler_64.c $(LDFLAGS) -o scaler_64
	echo "scaler & scaler_64"

fma: fma_detect.c
	$(CC) -mavx512f $^ -o fma_detect
	echo "fma_detect"

clean:
	rm -f avx2 avx2_64 avx512 scaler scaler_64 cube fma_detect *.o

