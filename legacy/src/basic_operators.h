#ifndef BASIC_OPERATORS_H
#define BASIC_OPERATORS_H

#include "grid.h"

typedef void (*CalcWSumFunc)(double* output,
                             const struct GridSubset *subset,
                             const struct GridConnectivity* connectivity,
                             const double *values,
                             const double *weights,
                             const size_t level_count);
void
calc_wsum_generic(double* output,
                  const struct GridSubset *subset,
                  const struct GridConnectivity* connectivity,
                  const double *values,
                  const double *weights,
                  const size_t level_count);
void
calc_wsum_triangle(double* output,
                   const struct GridSubset *subset,
                   const struct GridConnectivity* connectivity,
                   const double *values,
                   const double *weights,
                   const size_t level_count);

#endif // BASIC_OPERATORS_H

