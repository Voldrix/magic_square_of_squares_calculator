# magic square of squares calculator

### _Brute force a 3x3 magic square of squares extremely quickly_

Improving on the worst case, N^9 time complexity, this runs in (N^4)/(C*V) time.\
Where C is the number of CPU cores, and V is the vector width of the AVX registers, and N is the upper bound of the root numbers to check.

__Benchmark__\
The AVX2 version can check N = 624 in 1.7 sec, or N = 4096 in 30min on my 16 core Xeon @ 3.6GHz.

### How it Works
Brute forces the first 4 numbers (from 0..N^2), and calculates the remaining five based on the magic constant derived from the first 3 numbers.\
Then it must check if the 5 calculated values are perfect squares. Division and square roots are difficult for a computer and extremely slow, so we must use a trick. Luckily we know all perfect square numbers end in one of only 4 digits (of 16) in hexadecimal (0,1,4,9), so we can eliminate 75% of sqrt checks.

The code is small enough to fit in the processor's level 1 cache, negating any need for slow memory access.\
After it loads, the program generates no new variables/memory allocation while running, so it can take advantage of its few variables to keep them stored at the ready in the CPU registers.

### Files
- `scaler` multithreaded scalar version (no AVX)
- `avx2` 8 integer wide AVX2 instruction set (recommended)
- `avx512` 16 integer wide AVX512dq instructions
  - this is actually slower than AVX2 in most cases, unless your processor has 2 FMA units per core (Xeon Platinum)
- `cube` 3x3x3 [magic cube](https://en.wikipedia.org/wiki/Magic_cube) (not of squares). scalar
- `fma_detect` detect how many FMA units your processor has per core
  - there is no hardware flag for this feature, so this is based on Intel's recommended detection method, which is a benchmark test, so the results may not be perfect. check Intel Ark. mainly just Xeon Platinums have two

### How to Run
__Requirements:__ Linux & gcc\
Open the file you want to run, and set the 2 macros `CORES` and `UB` (upper bound of root numbers to check)\
`UB` must be a multiple of CORES __+ 1__.\
In the avx files, `UB` must be a multiple of cores AND vector width __+ 1__\
The 32-bit versions are faster, but can only count to 2 billion\
__Build:__
```
make (makes scaler + avx2)
make scaler / avx2 / avx512 / cube / fma
```
Squares with 7, 8, or 9 unique numbers will print to stdout.

## About

__Magic Square__\
A [magic square](https://en.wikipedia.org/wiki/Magic_square) is a square grid of numbers that add up to the same number in every column, row, and diagonal. A magic square of squares is just one where every number happens to be a perfect square (has an integer square root). The grid can be any (square) size, but this program only looks at 3x3 since the square of squares of order 3 remains unsolved.

__Parker Square__\
Matt Parker of [Stand-up Maths](https://www.youtube.com/user/standupmaths) "had a go" at solving one of these, and came up with one that wasn't quite right, didn't match the diagonal, and had a couple duplicate numbers, but has since become synonymous with giving a difficult problem your best go. sources [video1](https://www.youtube.com/watch?v=aOT_bG-vWyg), [video2](https://www.youtube.com/watch?v=FCczHiXPVcA)

This program can generate Parker squares as well, by just commenting out one of the constraint checks. If you remove just the second diagonal check, but still require all unique numbers, you'll actually generate a fair number of Parker squares almost immediately. The smallest of which being the five permutations of this one

| 94 | 97 | 58 |
|---|---|---|
| 2 | 74 | 127 |
| 113 | 82 | 46 |

which is the [Gardner square](https://en.wikipedia.org/wiki/Magic_square#Gardner_square)

### Contributing
If you can further optimize this code, feel free to send a PR.\
Make sure to benchmark your changes before submitting.\
Do not add any unnecessary code, such as automatic core count detection. Let's keep it minimal.

__License__\
[MIT License](LICENSE)
