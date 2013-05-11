// g++ native.cpp -lrt -o native

#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <vector>

#define ITERATIONS 10000000

int main(int argc, char *argv[])
{
  std::vector<unsigned long long> results;
  struct timespec start_time, end_time;
  struct stat output;
  int result;
  int fd;

  fd = open(argv[0], O_RDONLY);
  if (fd < 0) {
    return 1;
  }

  unsigned long long min = (unsigned long long) -1, max = 0, sum = 0;
  double avg, delta, variance = 0.0, sd;

  results.reserve(ITERATIONS);
  for (int i = 0; i < ITERATIONS; ++i) {
    if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0) {
      printf("Error calling clock_gettime\n");
      break;
    }
    result = __fxstat(3, fd, &output);
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

