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
for OMP_NUM_THREADS in $(seq $max_threads); do
  export OMP_NUM_THREADS

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

cd ..

