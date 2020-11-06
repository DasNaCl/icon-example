#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "utils.h"

#ifdef CPU_DEVICE
#include <be/cpu_threads.h>
#endif


size_t*
create_random_array_size_t(const size_t size,
                           const size_t min,
                           const size_t max)
{
  assert(max > min);

  size_t *array = (size_t*) malloc(sizeof(size_t) * size);

  for (size_t i = 0; i < size; ++i)
    array[i] = rand() % (max - min + 1) + min;

  return array;
}

double*
create_random_array_double(const size_t size,
                           const double min,
                           const double max)
{
  assert(max > min);

  double range = (max - min);
  double div = RAND_MAX / range;

  double *array = (double*) malloc(sizeof(double) * size);

  for (size_t i = 0; i < size; ++i)
  {
    array[i] = min + (rand() / div);
  }

  return array;
}

void
print_edge_connections(const struct Grid *grid,
                       const size_t edge_idx)
{
  printf("Edge: %zu", edge_idx);
  if (grid->coordinates[EDGE] != NULL)
  {
    printf(" (%f; %f)",
           grid->coordinates[EDGE][edge_idx].lat,
           grid->coordinates[EDGE][edge_idx].lon);
  }
  printf("\n");

  const struct GridConnectivity *e2v = grid->connectivities[EDGE][VERT],
                                *v2e = grid->connectivities[VERT][EDGE];

  for (size_t i = 0; i < e2v->tgt_count; ++i)
  {
    size_t vert_idx = e2v->matrix[e2v->tgt_count * edge_idx + i];
    printf("\tVertex: %zu", vert_idx);
    if (grid->coordinates[VERT] != NULL)
    {
      printf(" (%f; %f)",
             grid->coordinates[VERT][vert_idx].lat,
             grid->coordinates[VERT][vert_idx].lon);
    }
    printf("\n");

    printf("\t\tEdges:");
    for (size_t j = 0; j < v2e->tgt_count; ++j)
      printf(" %zu", v2e->matrix[v2e->tgt_count * vert_idx + j]);
    printf("\n");
  }

  const struct GridConnectivity *e2c = grid->connectivities[EDGE][CELL],
                                *c2e = grid->connectivities[CELL][EDGE];

  for (size_t i = 0; i < e2c->tgt_count; ++i)
  {
    size_t cell_idx = e2c->matrix[e2c->tgt_count * edge_idx + i];
    printf("\tCell: %zu", cell_idx);
    if (grid->coordinates[CELL] != NULL)
    {
      printf(" (%f; %f)",
             grid->coordinates[CELL][cell_idx].lat,
             grid->coordinates[CELL][cell_idx].lon);
    }
    printf("\n");

    printf("\t\tEdges:");
    for (size_t j = 0; j < c2e->tgt_count; ++j)
      printf(" %zu", c2e->matrix[c2e->tgt_count * cell_idx + j]);
    printf("\n");
  }
}

int64_t*
create_bench_array(const size_t size)
{
  return malloc(size * sizeof(uint64_t));
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

