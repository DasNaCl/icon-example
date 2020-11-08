#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netcdf.h>

#include "grid.h"

static const char* EDGE_DIM_NAME = "edge";
static const char* VERT_DIM_NAME = "vertex";
static const char* CELL_DIM_NAME = "cell";

static const char* EDGE_CONNECT_DIM_NAME = "nc";
static const char* VERT_CONNECT_DIM_NAME = "ne";
static const char* CELL_CONNECT_DIM_NAME = "nv";

static const char* EDGE_TO_VERT_VAR_NAME = "edge_vertices";
static const char* EDGE_TO_CELL_VAR_NAME = "adjacent_cell_of_edge";

static const char* VERT_TO_EDGE_VAR_NAME = "edges_of_vertex";
static const char* VERT_TO_VERT_VAR_NAME = "vertices_of_vertex";
static const char* VERT_TO_CELL_VAR_NAME = "cells_of_vertex";

static const char* CELL_TO_EDGE_VAR_NAME = "edge_of_cell";
static const char* CELL_TO_VERT_VAR_NAME = "vertex_of_cell";
static const char* CELL_TO_CELL_VAR_NAME = "neighbor_cell_index";

static const char* EDGE_LAT_VAR_NAME = "elat";
static const char* EDGE_LON_VAR_NAME = "elon";

static const char* VERT_LAT_VAR_NAME = "vlat";
static const char* VERT_LON_VAR_NAME = "vlon";

static const char* CELL_LAT_VAR_NAME = "clat";
static const char* CELL_LON_VAR_NAME = "clon";

static void check_io(const int status);

static size_t read_dim_size(const int nc_id,
                            const char *dim_name);

static int* read_array_int(const int nc_id,
                           const char *var_name,
                           const size_t size);

static double* read_array_double(const int nc_id,
                                 const char *var_name,
                                 const size_t size);

static struct GridConnectivity* read_grid_connectivity(const int nc_id,
                                                       const char* var_name,
                                                       const size_t src_count,
                                                       const size_t tgt_count);

static struct LatLonRad* read_coordinates(const int nc_id,
                                          const char *lat_var_name,
                                          const char *lon_var_name,
                                          const size_t size);

struct GridSubset*
alloc_grid_subset(const size_t count)
{
  struct GridSubset *subset = (struct GridSubset*) malloc(sizeof(struct GridSubset));
  subset->count = count;
  subset->indices = (size_t*) malloc(sizeof(size_t) * count);
  subset->grid = NULL;
  subset->entity = -1;

  return subset;
}

void
free_grid_subset(struct GridSubset *subset)
{
  if (subset == NULL)
    return;

  free(subset->indices);
  free(subset);
}

struct GridSubset*
create_grid_subset_all(const struct Grid *grid,
                       const enum GridEntity entity)
{
  return create_grid_subset_range(grid, entity,
                                  0, grid->counts[entity] - 1);
}

struct GridSubset*
create_grid_subset_range(const struct Grid *grid,
                         const enum GridEntity entity,
                         const size_t first_idx,
                         const size_t last_idx)
{
  assert(first_idx <= last_idx);
  assert(last_idx < grid->counts[entity]);

  const size_t count = last_idx - first_idx + 1;

  struct GridSubset *subset = alloc_grid_subset(count);

  subset->grid = grid;
  subset->entity = entity;
  for (size_t i = 0; i < count; ++i)
    subset->indices[i] = first_idx + i;

  return subset;
}

struct GridSubset*
create_grid_subset_region(const struct Grid *grid,
                          const enum GridEntity entity,
                          const struct LatLonRad south_west,
                          const struct LatLonRad north_east)
{
  size_t count = 0, alloc_size = 1;
  size_t *indices = (size_t*) malloc(sizeof(size_t) * alloc_size);

  const struct LatLonRad *entity_coords = grid->coordinates[entity];

  for (size_t i = 0; i < grid->counts[entity]; ++i)
  {
    struct LatLonRad coords = entity_coords[i];
    if (coords.lat >= south_west.lat && coords.lat <= north_east.lat &&
        coords.lon >= south_west.lon && coords.lon <= north_east.lon)
    {
      while (count >= alloc_size)
      {
        alloc_size <<= 1;
        indices = realloc(indices, sizeof(size_t) * alloc_size);
      }

      indices[count++] = i;
    }
  }

  struct GridSubset *subset = alloc_grid_subset(count);
  subset->grid = grid;
  subset->entity = entity;
  memcpy(subset->indices, indices, sizeof(size_t) * count);
  free(indices);

  return subset;
}

struct GridConnectivity*
alloc_grid_connectivity(const int src_count,
                        const int tgt_count)
{
  struct GridConnectivity *connectivity = (struct GridConnectivity*) malloc(sizeof(struct GridConnectivity));
  connectivity->src_count = src_count;
  connectivity->tgt_count = tgt_count;
  connectivity->matrix = (size_t*) malloc(sizeof(size_t) * src_count * tgt_count);

  return connectivity;
}

void
free_grid_connectivity(struct GridConnectivity *connectivity)
{
  if (connectivity == NULL)
    return;

  free(connectivity->matrix);
  free(connectivity);
}

struct Grid*
read_grid(const char *filename)
{
  int nc_id;

  check_io(nc_open(filename, NC_NOWRITE, &nc_id));

  struct Grid *grid = (struct Grid*) malloc(sizeof(struct Grid));

  grid->counts[EDGE] = read_dim_size(nc_id, EDGE_DIM_NAME);
  grid->counts[VERT] = read_dim_size(nc_id, VERT_DIM_NAME);
  grid->counts[CELL] = read_dim_size(nc_id, CELL_DIM_NAME);

  // Read and check the edge connectivity, i.e. the number of vertices
  // connected by one edge, as well as the number of cells that share an edge.
  // The value equals to 2 by definition:
  size_t edge_connect = read_dim_size(nc_id, EDGE_CONNECT_DIM_NAME);
  assert (edge_connect == 2);

  // Read the maximum vertex connectivity, i.e. maximum the number of
  // edges/vertices/cells that share a vertex. In the case of an icosahedral
  // grid, the value is 6, but we would like to be able to work with other
  // types of unstructured grids, which might have other values.
  size_t vert_connect = read_dim_size(nc_id, VERT_CONNECT_DIM_NAME);

  // Read the maximum cell connectivity, i.e. the maximum number of
  // edges and vertices in a cell, as well as the maximum number of cells that
  // are neighbours for a given cell.
  size_t cell_connect = read_dim_size(nc_id, CELL_CONNECT_DIM_NAME);

  // Read edge connectivity matrices.
  grid->connectivities[EDGE][EDGE] = NULL;

  grid->connectivities[EDGE][VERT] =
    read_grid_connectivity(nc_id, EDGE_TO_VERT_VAR_NAME,
                           grid->counts[EDGE], edge_connect);

  grid->connectivities[EDGE][CELL] =
    read_grid_connectivity(nc_id, EDGE_TO_CELL_VAR_NAME,
                           grid->counts[EDGE], edge_connect);

  // Read vertex connectivity matrices.
  grid->connectivities[VERT][EDGE] =
    read_grid_connectivity(nc_id, VERT_TO_EDGE_VAR_NAME,
                           grid->counts[VERT], vert_connect);

  grid->connectivities[VERT][VERT] =
    read_grid_connectivity(nc_id, VERT_TO_VERT_VAR_NAME,
                           grid->counts[VERT], vert_connect);

  grid->connectivities[VERT][CELL] =
    read_grid_connectivity(nc_id, VERT_TO_CELL_VAR_NAME,
                           grid->counts[VERT], vert_connect);

  // Read cell connectivity matrices.
  grid->connectivities[CELL][EDGE] =
    read_grid_connectivity(nc_id, CELL_TO_EDGE_VAR_NAME,
                           grid->counts[CELL], cell_connect);

  grid->connectivities[CELL][VERT] =
    read_grid_connectivity(nc_id, CELL_TO_VERT_VAR_NAME,
                           grid->counts[CELL], cell_connect);

  grid->connectivities[CELL][CELL] =
    read_grid_connectivity(nc_id, CELL_TO_CELL_VAR_NAME,
                           grid->counts[CELL], cell_connect);

  // Read edge coordinates.
  grid->coordinates[EDGE] = read_coordinates(nc_id,
                                             EDGE_LAT_VAR_NAME,
                                             EDGE_LON_VAR_NAME,
                                             grid->counts[EDGE]);

  // Read vertex coordinates.
  grid->coordinates[VERT] = read_coordinates(nc_id,
                                             VERT_LAT_VAR_NAME,
                                             VERT_LON_VAR_NAME,
                                             grid->counts[VERT]);

  // Read cell coordinates.
  grid->coordinates[CELL] = read_coordinates(nc_id,
                                             CELL_LAT_VAR_NAME,
                                             CELL_LON_VAR_NAME,
                                             grid->counts[CELL]);

  check_io(nc_close(nc_id));

  return grid;
}

void
free_grid(struct Grid *grid)
{
  if (grid == NULL)
    return;

  for (int i = 0; i < GRID_ENTITY_ENUM_COUNT; ++i)
  {
    for (int j = 0; j < GRID_ENTITY_ENUM_COUNT; ++j)
      free_grid_connectivity(grid->connectivities[i][j]);

    free(grid->coordinates[i]);
  }

  free(grid);
}

struct Grid*
create_tetrahedron_grid()
{
  struct Grid *grid = (struct Grid*) malloc(sizeof(struct Grid));

  grid->counts[EDGE] = 6;
  grid->counts[VERT] = 4;
  grid->counts[CELL] = 4;

  size_t edge_connect = 2;
  size_t vert_connect = 3;
  size_t cell_connect = 3;

  grid->connectivities[EDGE][EDGE] = NULL;

  const size_t edge_to_vert[] = {0,1, 1,3, 0,3, 2,3, 0,2, 1,2};
  grid->connectivities[EDGE][VERT] =
    alloc_grid_connectivity(grid->counts[EDGE],
                            edge_connect);
  memcpy(grid->connectivities[EDGE][VERT]->matrix,
         edge_to_vert, sizeof(edge_to_vert));

  const size_t edge_to_cell[] = {0,3, 0,2, 0,1, 1,2, 1,3, 2,3};
  grid->connectivities[EDGE][CELL] =
    alloc_grid_connectivity(grid->counts[EDGE],
                            edge_connect);
  memcpy(grid->connectivities[EDGE][CELL]->matrix,
         edge_to_cell, sizeof(edge_to_cell));

  const size_t vert_to_edge[] = {0,2,4, 0,1,5, 3,4,5, 1,2,3};
  grid->connectivities[VERT][EDGE] =
    alloc_grid_connectivity(grid->counts[VERT],
                            vert_connect);
  memcpy(grid->connectivities[VERT][EDGE]->matrix,
         vert_to_edge, sizeof(vert_to_edge));

  const size_t vert_to_vert[] = {1,2,3, 0,2,3, 0,1,3, 0,1,2};
  grid->connectivities[VERT][VERT] =
    alloc_grid_connectivity(grid->counts[VERT],
                            vert_connect);
  memcpy(grid->connectivities[VERT][VERT]->matrix,
         vert_to_vert, sizeof(vert_to_vert));

  const size_t vert_to_cell[] = {0,1,3, 0,2,3, 1,2,3, 0,1,2};
  grid->connectivities[VERT][CELL] =
    alloc_grid_connectivity(grid->counts[VERT],
                            vert_connect);
  memcpy(grid->connectivities[VERT][CELL]->matrix,
         vert_to_cell, sizeof(vert_to_cell));

  const size_t cell_to_edge[] = {0,1,2, 2,3,4, 1,3,5, 0,4,5};
  grid->connectivities[CELL][EDGE] =
    alloc_grid_connectivity(grid->counts[CELL],
                            cell_connect);
  memcpy(grid->connectivities[CELL][EDGE]->matrix,
         cell_to_edge, sizeof(cell_to_edge));

  const size_t cell_to_vert[] = {0,1,3, 0,2,3, 1,2,3, 0,1,2};
  grid->connectivities[CELL][VERT] =
    alloc_grid_connectivity(grid->counts[CELL],
                            cell_connect);
  memcpy(grid->connectivities[CELL][VERT]->matrix,
         cell_to_vert, sizeof(cell_to_vert));

  const size_t cell_to_cell[] = {1,2,3, 0,2,3, 0,1,3, 0,1,2};
  grid->connectivities[CELL][CELL] =
    alloc_grid_connectivity(grid->counts[CELL],
                            cell_connect);
  memcpy(grid->connectivities[CELL][CELL]->matrix,
         cell_to_cell, sizeof(cell_to_cell));

  for (int i = 0; i < GRID_ENTITY_ENUM_COUNT; ++i)
    grid->coordinates[i] = NULL;

  return grid;
}

static void
check_io(const int status)
{
  if (status != NC_NOERR)
  {
    fprintf(stderr, "NetCDF: %s\n", nc_strerror(status));
    exit(-1);
  }
}

static size_t
read_dim_size(const int nc_id,
              const char *dim_name)
{
  int dim_id;
  check_io(nc_inq_dimid(nc_id, dim_name, &dim_id));

  size_t size;
  check_io(nc_inq_dimlen(nc_id, dim_id, &size));

  return size;
}

static int*
read_array_int(const int nc_id,
               const char *var_name,
               const size_t size)
{
  int var_id;
  check_io(nc_inq_varid(nc_id, var_name, &var_id));

  int *array = (int*) malloc(sizeof(int) * size);
  check_io(nc_get_var_int(nc_id, var_id, array));

  return array;
}

static double*
read_array_double(const int nc_id,
                  const char *var_name,
                  const size_t size)
{
  int var_id;
  check_io(nc_inq_varid(nc_id, var_name, &var_id));

  double *array = (double*) malloc(sizeof(double) * size);
  check_io(nc_get_var_double(nc_id, var_id, array));

  return array;
}

static struct GridConnectivity*
read_grid_connectivity(const int nc_id,
                       const char* var_name,
                       const size_t src_count,
                       const size_t tgt_count)
{
  int *int_buf = read_array_int(nc_id, var_name, src_count * tgt_count);

  struct GridConnectivity *connectivity = alloc_grid_connectivity(src_count, tgt_count);

  for (size_t i = 0; i < tgt_count; ++i)
  {
    for (size_t j = 0; j < src_count; ++j)
      connectivity->matrix[j * tgt_count + i] = int_buf[i * src_count + j] - 1;
  }

  free(int_buf);

  return connectivity;
}

static struct LatLonRad*
read_coordinates(const int nc_id,
                 const char *lat_var_name,
                 const char *lon_var_name,
                 const size_t size)
{
  double *lats = read_array_double(nc_id, lat_var_name, size);
  double *lons = read_array_double(nc_id, lon_var_name, size);

  struct LatLonRad *coordinates = (struct LatLonRad*) malloc(sizeof(struct LatLonRad) * size);

  for (size_t i = 0; i < size; ++i)
  {
    coordinates[i].lat = lats[i];
    coordinates[i].lon = lons[i];
  }

  free(lats);
  free(lons);

  return coordinates;
}
