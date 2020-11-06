#pragma once

#include <stdlib.h>
#include <grid.h>

typedef void (*CalcWSumFunc)(double*,
                             const struct GridSubset*,
                             const struct GridConnectivity*,
                             const double*,
                             const double*,
                             const uint32_t);

extern
void calc_wsum_generic(double* output,
                       const struct GridSubset *subset,
                       const struct GridConnectivity* connectivity,
                       const double *values,
                       const double *weights,
                       const uint32_t level_count);

extern
void calc_wsum_triangle(double* output,
                        const struct GridSubset *subset,
                        const struct GridConnectivity* connectivity,
                        const double *values,
                        const double *weights,
                        const uint32_t level_count);
