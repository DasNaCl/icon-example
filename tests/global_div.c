#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <anydsl.h>
#include <grid.h>
#include <utils.h>

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s grid_filename level_count [generic|triangle]\n", argv[0]);
    exit(-1);
  }

  const char *grid_filename = argv[1];
  printf("Grid file: %s\n", grid_filename);

  struct Grid* grid = read_grid(grid_filename);
  printf("# edges: %zu\n", grid->counts[EDGE]);
  printf("# vertices: %zu\n", grid->counts[VERT]);
  printf("# cells: %zu\n", grid->counts[CELL]);

  const size_t level_count = atoi(argv[2]);
  printf("# levels: %zu\n", level_count);

#ifdef _OPENMP
  printf("# OpenMP threads: %i\n", omp_get_max_threads());
#endif // _OPENMP

  CalcWSumFunc calc_wsum_function = NULL;
  if (argc >= 4 && strcmp(argv[3], "triangle") == 0)
  {
    calc_wsum_function = &calc_wsum_triangle;
    printf("Weighted sum function: triangle\n");
  }
  else
  {
    calc_wsum_function = &calc_wsum_generic;
    printf("Weighted sum function: generic\n");
  }

  /////////////////////

  double *div_var = (double*) malloc(sizeof(double) * grid->counts[CELL] * level_count);
  double *flux_var = create_random_array_double(grid->counts[EDGE] * level_count, 0, 100);
  double *div_factors = create_random_array_double(grid->counts[CELL] * grid->connectivities[CELL][EDGE]->tgt_count, 0.2, 0.4);
  struct GridSubset *subset = create_grid_subset_all(grid, CELL);

  struct timespec start, finish;
  clock_gettime(CLOCK_MONOTONIC, &start);

  calc_wsum_function(div_var, subset, grid->connectivities[CELL][EDGE], flux_var, div_factors, level_count);

  clock_gettime(CLOCK_MONOTONIC, &finish);

  double elapsed_ms = (finish.tv_sec - start.tv_sec) * 1000.0 + (finish.tv_nsec - start.tv_nsec) * 1e-6;

  printf("Operation time, ms: %f\n", elapsed_ms);

  /*
  FILE* f = fopen("output", "w");
  for(size_t i = 0; i < grid->counts[CELL] * level_count; ++i)
  {
    fprintf(f, "%f\n", div_var[i]);
  }
  fclose(f);
  */

  free_grid_subset(subset);
  free_grid(grid);
  free(div_var);
  free(flux_var);
  free(div_factors);

  return 0;
}

