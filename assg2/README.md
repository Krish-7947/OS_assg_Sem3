# Multithreaded Perfect Number Checker

A C program that determines whether a given number is perfect using POSIX threads.

A number **N** is perfect if the sum of its proper divisors (excluding itself) equals N.  
Example: `28 = 1 + 2 + 4 + 7 + 14`

---

## Compilation

```bash
gcc -o perfect perfect.c -lpthread -lm
```

## Usage

```bash
./perfect {A number} {Number of threads}
```

## Example

```bash
$ ./perfect 28 4
28 is a Perfect Number

$ ./perfect 10 2
10 is not a Perfect Number
```

---

## How It Works

The range `1` to `sqrt(N)` is divided into `P` equal subranges, one per thread. Each thread checks whether values in its range divide N evenly. When a divisor `i` is found, both `i` and `N/i` are added to a shared sum — since divisors come in pairs. A mutex lock protects the shared variable from concurrent writes.

Once all threads finish, the sum is compared with N to determine the result.

**Optimization:** Iterating only up to `sqrt(N)` reduces the time complexity from O(N) to O(sqrt(N)/P).

---


## Dependencies

- GCC
- POSIX threads (`pthreads`)
- Math library (`libm`)