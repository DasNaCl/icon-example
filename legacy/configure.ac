AC_INIT([icon-example], [1.1])
AC_PREREQ([2.69])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([src/grid.c])
AC_CONFIG_HEADERS([config/config.h])

AM_INIT_AUTOMAKE([1.16.1 foreign])
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_CC_C99
AC_ARG_VAR([AR], [archiver command])
AC_CHECK_TOOL([AR], [ar], [:])
AS_VAR_IF([AR], [:], [AC_MSG_ERROR([the archiver command not found])])
AC_ARG_VAR([ARFLAGS], [archiver flags])
ARFLAGS=${ARFLAGS-cr}
AC_ARG_VAR([RANLIB], [archive indexer command])
AC_PROG_RANLIB

AC_ARG_ENABLE([rpaths],
  [AC_HELP_STRING([--enable-rpaths],
     [add directories specified with -L flags in LDFLAGS and LIBS to the
runtime library search paths (RPATH) @<:@default=yes@:>@])], [],
  [enable_rpaths=yes])

AS_VAR_IF([enable_rpaths], [yes],
  [ASX_EXTRACT_ARGS([acx_L_args], ["$LDFLAGS $LIBS"], ['-L@<:@ @:>@*'])
   for acx_L_arg in $acx_L_args; do
     ASX_VAR_APPEND_UNIQ([LDFLAGS],
       ["-Wl,-rpath -Wl,$acx_L_arg"], [' '])
   done
   ACX_SHLIB_RPATH_FLAGS_CHECK])

AC_ARG_ENABLE([openmp],
  [AC_HELP_STRING([--enable-openmp],
     [enable OpenMP support @<:@default: yes@:>@])], [],
  [enable_openmp=yes])
AC_SUBST([OPENMP_CFLAG], [''])
AM_CONDITIONAL([OPENMP_ENABLED], [test x"$enable_openmp" = xyes])

AS_VAR_IF([enable_openmp], [yes],
  [ACX_LANG_OPENMP_FLAG([OPENMP_CFLAG=$acx_cv_c_openmp_flag])],
  [ACX_LANG_MACRO_CHECK_DEFINED([_OPENMP])
   AS_VAR_IF([acx_macro_defined], [yes],
     [AC_MSG_ERROR([C compiler enables OpenMP support by default: set dnl
CFLAGS accordingly to disable it])])])

AC_SEARCH_LIBS([clock_gettime], [rt], [],
  [AC_MSG_FAILURE([cannot find a library defining function clock_gettime])])

AC_ARG_WITH([netcdf],
  [AC_HELP_STRING([--with-netcdf=<yes|directory>],
     [location of NetCDF library (lib and include subdirs)])],
  [AS_VAR_IF([withval], [no],
     [AC_MSG_ERROR([cannot build the package without NetCDF library])])],
  [with_netcdf=yes])
AC_ARG_VAR([NETCDF_CFLAGS],
  [exact C compiler flags enabling NetCDF])
AC_ARG_VAR([NETCDF_CLIBS],
  [exact linker flags enabling NetCDF when linking with C compiler])

acx_inc_search_args=
acx_lib_search_args=
AS_IF([test x"$with_netcdf" != xyes],
  [acx_inc_search_args="-I$with_netcdf/include"
   acx_lib_search_args="-L$with_netcdf/lib"
   AS_VAR_IF([enable_rpaths], [yes],
     [AS_VAR_APPEND([acx_lib_search_args],
        [" -Wl,-rpath -Wl,$with_netcdf/lib"])])])

acx_save_CFLAGS=$CFLAGS
NETCDF_CFLAGS=${NETCDF_CFLAGS-$acx_inc_search_args}
CFLAGS="$NETCDF_CFLAGS $acx_save_CFLAGS"
AC_CHECK_HEADER([netcdf.h],
  [ACX_LANG_LIB_SEARCH([NETCDF_CLIBS], [nc_open],
     [ASX_PREPEND_LDFLAGS([$acx_lib_search_args], [-lnetcdf])],
     [NETCDF_CLIBS=$acx_cv_c_lib_func_nc_open],
     [AC_MSG_FAILURE([cannot link to NetCDF library])])],
  [AC_MSG_FAILURE([netcdf.h not found])], [//])
CFLAGS=$acx_save_CFLAGS

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])

AC_CONFIG_FILES([
  tests/global_div.benchmark
  tests/global_div.run
  tests/local_vel_filter.run
], [chmod a+x "$ac_file"])

AC_OUTPUT
