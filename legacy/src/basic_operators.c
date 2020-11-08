#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "grid.h"

#include "basic_operators.h"

void
calc_wsum_generic_real(double* output,
                  const struct GridSubset* subset,
                  const struct GridConnectivity* connectivity,
                  const double *values,
                  const double *weights,
                  const size_t level_count)
{
#pragma omp parallel for
  for (size_t output_idx = 0; output_idx < subset->count; ++output_idx)
  {
    const size_t entity_idx = subset->indices[output_idx];
    for (size_t level_idx = 0; level_idx < level_count; ++level_idx)
    {
      double result = 0.0;
      for (size_t connect_idx = 0; connect_idx < connectivity->tgt_count; ++connect_idx)
      {
        const size_t flat_connect_idx = entity_idx * connectivity->tgt_count + connect_idx;
        const size_t flat_values_idx = connectivity->matrix[flat_connect_idx] * level_count + level_idx;

        result += values[flat_values_idx] * weights[flat_connect_idx];
      }

      output[output_idx * level_count + level_idx] = result;
    }
  }
}

void
calc_wsum_triangle_real(double* output,
                   const struct GridSubset* subset,
                   const struct GridConnectivity* connectivity,
                   const double *values,
                   const double *weights,
                   const size_t level_count)
{
  assert(connectivity->tgt_count == 3);

#pragma omp parallel for
  for (size_t output_idx = 0; output_idx < subset->count; ++output_idx)
  {
    const size_t entity_idx = subset->indices[output_idx];
    const size_t first_connect_idx = entity_idx * connectivity->tgt_count;
    const size_t *values_idcs = &connectivity->matrix[first_connect_idx];

    for (size_t level_idx = 0; level_idx < level_count; ++level_idx)
    {
      output[output_idx * level_count + level_idx] =
        values[values_idcs[0] * level_count + level_idx] * weights[first_connect_idx + 0] +
        values[values_idcs[1] * level_count + level_idx] * weights[first_connect_idx + 1] +
        values[values_idcs[2] * level_count + level_idx] * weights[first_connect_idx + 2];
    }
  }
}


int
anydsl_is_profiling()
{
  const char* env_var = getenv("ANYDSL_PROFILE");
  if (env_var) {
    return strcasecmp(env_var, "full");
  }
  return -1;
}

static const size_t iter_cpu = 250;

int
compare_d (const void *a, const void *b)
{
  double _a = *((double*) a);
  double _b = *((double*) b);

       if (_a == _b) return  0;
  else if (_a <  _b) return -1;
  else               return  1;
}

void
benchmark(CalcWSumFunc func,
          double* output,
          const struct GridSubset* subset,
          const struct GridConnectivity* connectivity,
          const double *values,
          const double *weights,
          const size_t level_count)
{
  double* times = malloc (iter_cpu * sizeof(double));
  for (size_t i = 0; i < iter_cpu; ++i)
  {
    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);

    func (output, subset, connectivity, values, weights, level_count);

    clock_gettime(CLOCK_MONOTONIC, &finish);
    times[i] = (finish.tv_sec - start.tv_sec) * 1000. + (finish.tv_nsec - start.tv_nsec) / 1000000.;
  }
  qsort(times, iter_cpu, sizeof(double), compare_d);

  printf("Timing: %f | %f | %f (median(%d) | minimum | maximum) ms\n",
          times[iter_cpu/2],
          times[0],
          times[iter_cpu - 1],
          iter_cpu);

  free (times);
}

void
calc_wsum_generic(double* output,
                  const struct GridSubset* subset,
                  const struct GridConnectivity* connectivity,
                  const double *values,
                  const double *weights,
                  const size_t level_count)
{
  if (anydsl_is_profiling() == 0)
    benchmark (calc_wsum_generic_real, output, subset, connectivity, values, weights, level_count);
  else
    calc_wsum_generic_real(output, subset, connectivity, values, weights, level_count);
}

void
calc_wsum_triangle(double* output,
                   const struct GridSubset* subset,
                   const struct GridConnectivity* connectivity,
                   const double *values,
                   const double *weights,
                   const size_t level_count)
{
  if (anydsl_is_profiling() == 0)
    benchmark (calc_wsum_triangle_real, output, subset, connectivity, values, weights, level_count);
  else
    calc_wsum_triangle_real(output, subset, connectivity, values, weights, level_count);
}
