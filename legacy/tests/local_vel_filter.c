#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

#include <grid.h>
#include <utils.h>
#include <basic_operators.h>

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s grid_filename level_count\n", argv[0]);
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

  /////////////////////

  const struct GridConnectivity *e2c = grid->connectivities[EDGE][CELL],
                                *c2e = grid->connectivities[CELL][EDGE];

  // Create connectivity that maps an edge E to all edges that belong to
  // the cells that E belongs to.
  struct GridConnectivity *e2e =
    alloc_grid_connectivity(grid->counts[EDGE], e2c->tgt_count * c2e->tgt_count);

  for (size_t e_src_idx = 0; e_src_idx < e2e->src_count; ++e_src_idx)
  {
    for (size_t c_indir_idx = 0; c_indir_idx < e2c->tgt_count; ++c_indir_idx)
    {
      const size_t c_idx = e2c->matrix[e2c->tgt_count * e_src_idx + c_indir_idx];

      for (size_t e_tgt_indir_idx = 0; e_tgt_indir_idx < c2e->tgt_count; ++e_tgt_indir_idx)
      {
        e2e->matrix[e2e->tgt_count * e_src_idx + c2e->tgt_count * c_indir_idx + e_tgt_indir_idx] =
          c2e->matrix[c2e->tgt_count * c_idx + e_tgt_indir_idx];
      }
    }
  }

  // Each edge has the same weight.
  double *weights = (double*) malloc(sizeof(double) * e2e->src_count * e2e->tgt_count);
  const double common_weight = 1. / e2e->tgt_count;

  for (size_t e_tgt_indir_idx = 0; e_tgt_indir_idx < e2e->tgt_count; ++e_tgt_indir_idx)
    weights[e_tgt_indir_idx] = common_weight;

  for (size_t e_src_indir_idx = 1; e_src_indir_idx < e2e->src_count; ++e_src_indir_idx)
    memcpy(&weights[e2e->tgt_count * e_src_indir_idx], &weights[0], sizeof(double) * e2e->tgt_count);

  // Select a region of interest.
  const struct LatLonRad south_west = {-M_PI / 4.0, -M_PI / 2.0},
                         north_east = { M_PI / 4.0,  M_PI / 2.0};

  struct GridSubset *subset = create_grid_subset_region(grid, EDGE, south_west, north_east);

  // Input velocities.
  double *input_vels = create_random_array_double(grid->counts[EDGE] * level_count, 0, 10);

  double *output_vels = (double*) malloc(sizeof(double) * subset->count * level_count);

  calc_wsum_generic(output_vels, subset, e2e, input_vels, weights, level_count);

  free_grid(grid);
  free_grid_connectivity(e2e);
  free_grid_subset(subset);
  free(weights);
  free(input_vels);
  free(output_vels);

  return 0;
}

