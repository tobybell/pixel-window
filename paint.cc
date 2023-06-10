extern "C" {
#include "header.h"
}

#include <ctime>
#include <cstdio>
#include <cstdlib>

constexpr unsigned square_size = 32;

void paint(unsigned* data, unsigned width, unsigned height, unsigned row) {
  unsigned long start = clock();
  
  unsigned i0 = rand() % (height - square_size);
  unsigned j0 = rand() % (width - square_size);
  
  for (unsigned i = i0; i < i0 + square_size; ++i)
    for (unsigned j = j0; j < j0 + square_size; ++j)
      data[i * row + j] = (rand() % 255) << 8 | 255;
  printf("rendered %lu\n", clock() - start);
}
