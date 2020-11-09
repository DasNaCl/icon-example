#!/usr/bin/env sh

grid=build/src/icon_grid_0019_R02B05_G.nc
lvl_count=40
max_threads=$(nproc)
max_threads=$(( max_threads / 2 ))
[[ -z "${ICONEX_BENCH_NUM_ITER}" ]] && num_iter=1 || num_iter=$ICONEX_BENCH_NUM_ITER

echo "backend median minimum maximum generic threads"
for no in $(seq $num_iter); do
  for OMP_NUM_THREADS in $(seq $max_threads); do
    export OMP_NUM_THREADS

    for div_function_name in 'generic' 'triangle'; do
      echo -ne "cpu "

      ANYDSL_PROFILE=full './legacy/tests/global_div' "${grid}" "${lvl_count}" "${div_function_name}" | \
        awk '/Timing: /''{ printf "%.2f %.2f %.2f ", $2, $4, $6 }' || exit 1

      if [ $div_function_name = "generic" ]; then
        echo -ne "1 "
      else
        echo -ne "0 "
      fi
      echo -ne "$OMP_NUM_THREADS\n"
    done
  done
done

