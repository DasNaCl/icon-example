bin_PROGRAMS =     \
  global_div       \
  local_vel_filter

TESTS =                \
  global_div.run       \
  local_vel_filter.run \
  global_div.benchmark

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = $(OPENMP_CFLAG)
LDADD = $(top_builddir)/src/libicon_example.a $(NETCDF_CLIBS)

GRID_URL = http://icon-downloads.mpimet.mpg.de/grids/public/mpim/0019/icon_grid_0019_R02B05_G.nc
GRID_FILE = grid.nc

$(GRID_FILE):
	test -f "$${ICON_GRID}" || curl --fail --location --progress-bar --output $@ $(GRID_URL)

$(TESTS):| $(GRID_FILE)

DISTCLEANFILES = $(GRID_FILE)

AUTOMAKE_OPTIONS = serial-tests

