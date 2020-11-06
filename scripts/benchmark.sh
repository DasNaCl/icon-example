#!/usr/bin/env sh

[[ -z "${ICONEX_BENCH}" ]] && source ./bench_env.sh

grid=src/icon_grid_0019_R02B05_G.nc
lvl_count=40
max_threads=$(nproc)
max_threads=$(( max_threads / 2 ))

mkdir -p bench
cd bench

backends="$ICONEX_BENCH"

echo -ne "Benchmarking backends: $backends\n\n"

echo "backend median minimum maximum generic threads"
# Run cpu based backends
for backend in $backends; do
  if [ $backend = "cpu" ] || [ $backend = "avx" ]; then
    for t in $(seq $max_threads); do
      cmake ../.. -DAnyDSL_runtime_DIR=$PWD/../../../runtime/build/share/anydsl/cmake/ \
        -DBACKEND=$backend \
        -DNO_OF_THREADS=$t >/dev/null 2>&1

      make -j"$max_threads" >/dev/null 2>&1

      for div_function_name in 'generic' 'triangle'; do
        echo -ne "$backend "

        ANYDSL_PROFILE=full './global_div' "${grid}" "${lvl_count}" "${div_function_name}" | \
          awk '/Timing: /''{ printf "%.2f %.2f %.2f ", $2, $4, $6 }' || exit 1

        if [ $div_function_name = "generic" ]; then
          echo -ne "1 "
        else
          echo -ne "0 "
        fi
        echo -ne "$t\n"
      done
    done
  else
    cmake ../.. -DAnyDSL_runtime_DIR=$PWD/../../../runtime/build/share/anydsl/cmake/ \
      -DBACKEND=$backend >/dev/null 2>&1

    make -j"$max_threads" >/dev/null 2>&1

    for div_function_name in 'generic' 'triangle'; do
      echo -ne "$backend "

      ANYDSL_PROFILE=full './global_div' "${grid}" "${lvl_count}" "${div_function_name}" | \
        awk '/Timing: /''{ printf "%.2f %.2f %.2f ", $2, $4, $6 }' || exit 1

      if [ $div_function_name = "generic" ]; then
        echo -ne "1 1"
      else
        echo -ne "0 1"
      fi
      echo -ne "\n"
    done
  fi
done

cd ..

