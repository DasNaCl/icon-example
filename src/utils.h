#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#include "grid.h"

size_t*
create_random_array_size_t(const size_t size,
                           const size_t min,
                           const size_t max);

double*
create_random_array_double(const size_t size,
                           const double min,
                           const double max);

void
print_edge_connections(const struct Grid *grid,
                       const size_t edge_idx);


int64_t*
create_bench_array(const size_t size);

int
anydsl_is_profiling();

#endif // UTILS_H
