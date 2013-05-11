// g++ ffi.cpp -lffi -lrt -o ffi

#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ffi.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <vector>

#define ITERATIONS 10000000

int main(int argc, char *argv[])
{
  std::vector<unsigned long long> results;
  struct timespec start_time, end_time;
  ffi_cif cif;
  int result;
  struct stat output;
  int fd = open(argv[0], O_RDONLY);
  if (fd < 0) {
    return 1;
  }

  ffi_type  *args[] = { &ffi_type_sint, &ffi_type_sint, &ffi_type_pointer };
  int ver = 3;

  if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, sizeof(args)/sizeof(ffi_type*),
      &ffi_type_sint, args) != FFI_OK) {
    close(fd);
    return 1;
  }

  unsigned long long min = (unsigned long long) -1, max = 0, sum = 0;
  double avg, delta, variance = 0.0, sd;

  results.reserve(ITERATIONS);
  for(int i = 0; i < ITERATIONS; ++i) {
    void *values[] = { &ver, &fd, &output };
    if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0) {
      printf("Error calling clock_gettime\n");
      break;
    }
    ffi_call(&cif, reinterpret_cast<void(*)()>(&__fxstat), &result, values);
    if (clock_gettime(CLOCK_MONOTONIC, &end_time) != 0) {
      printf("Error calling clock_gettime\n");
      break;
    }
    unsigned long long start = static_cast<unsigned long long>(start_time.tv_sec) * 1000000000ULL + 
                    static_cast<unsigned long long>(start_time.tv_nsec);
    unsigned long long end = static_cast<unsigned long long>(end_time.tv_sec) * 1000000000ULL + 
                    static_cast<unsigned long long>(end_time.tv_nsec);
    unsigned long long ns = end - start;
    min = std::min(min, ns);
    max = std::max(max, ns);
    sum += ns;
    results.push_back(ns);
  }
  close(fd);

  avg = ((double)sum)/((double)results.size());
  for(std::vector<unsigned long long>::const_iterator itr = results.begin();
      itr != results.end(); ++itr) {
    delta = ((double)(*itr)) - avg;
    variance += delta * delta;
  }
  sd = sqrt(variance / ((double)results.size()));
  printf("Min: %llu ns, Max: %llu ns, Avg: %f ns, Std Dev: %f\n",
         min, max, avg, sd);
  return 0;
}

