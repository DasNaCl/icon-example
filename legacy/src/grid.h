#ifndef GRID_H
#define GRID_H

#include <stdlib.h>

enum GridEntity
{
  EDGE,
  VERT,
  CELL,
  GRID_ENTITY_ENUM_COUNT // number of real elements in enum
};

struct GridSubset
{
  size_t count;
  size_t *indices;
  const struct Grid *grid;
  enum GridEntity entity;
};

struct GridConnectivity
{
  size_t src_count,
         tgt_count;

  size_t *matrix;
};

struct LatLonRad
{
  double lat,
         lon;
};

struct Grid
{
  size_t counts[GRID_ENTITY_ENUM_COUNT];
  struct GridConnectivity *connectivities[GRID_ENTITY_ENUM_COUNT][GRID_ENTITY_ENUM_COUNT];
  struct LatLonRad *coordinates[GRID_ENTITY_ENUM_COUNT];
};

struct GridSubset*
alloc_grid_subset(const size_t count);

void
free_grid_subset(struct GridSubset *subset);

struct GridSubset*
create_grid_subset_all(const struct Grid *grid,
                       const enum GridEntity entity);

struct GridSubset*
create_grid_subset_range(const struct Grid *grid,
                         const enum GridEntity entity,
                         const size_t first_idx,
                         const size_t last_idx);

struct GridSubset*
create_grid_subset_region(const struct Grid *grid,
                          const enum GridEntity entity,
                          const struct LatLonRad south_west,
                          const struct LatLonRad north_east);

struct GridConnectivity*
alloc_grid_connectivity(const int src_count,
                        const int tgt_count);

void
free_grid_connectivity(struct GridConnectivity *connectivity);

struct Grid*
read_grid(const char *filename);

void
free_grid(struct Grid *grid);

// Creates a simple grid for debugging:
struct Grid*
create_tetrahedron_grid();

#endif // GRID_H

