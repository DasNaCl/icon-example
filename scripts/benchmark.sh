#!/usr/bin/env sh

grid=src/icon_grid_0019_R02B05_G.nc
lvl_count=40
iter_count=50
max_threads=$(nproc)
max_threads=$(( max_threads / 2 ))

mkdir -p bench
cd bench

backends="cpu"
[[ -z "${ICONEX_BENCH}" ]] || backends="$backends $ICONEX_BENCH"

echo "Benchmarking backends: $backends"

# Run cpu based backends
for backend in $backends; do
  printf -- "--------------------- %6s -\n" "${backend}"

  if [ $backend = "cpu" ] || [ $backend = "avx" ]; then
    for t in $(seq $max_threads); do
      cmake ../.. -DAnyDSL_runtime_DIR=$PWD/../../../runtime/build/share/anydsl/cmake/ \
        -DBACKEND=$backend \
        -DNO_OF_THREADS=$t >/dev/null 2>&1

      make -j"$max_threads" >/dev/null 2>&1

      printf -- "-- [Threads: %2s] -------------\n" "$t"

      for div_function_name in 'generic' 'triangle'; do
        echo "Div function name: ${div_function_name}"
        { for _ in $(seq ${iter_count}); do
            './global_div' "${grid}" "${lvl_count}" "${div_function_name}" || exit 1
          done } | awk '/Operation time, ms: / {sum+=$4; count+=1}END{print "Total runs:", count, "\nAverage time:", sum/count, "ms"}' || exit 1
        echo "------------------------------"
      done
    done
  else
    cmake ../.. -DAnyDSL_runtime_DIR=$PWD/../../../runtime/build/share/anydsl/cmake/ \
      -DBACKEND=$backend >/dev/null 2>&1

    make -j"$max_threads" >/dev/null 2>&1

    for div_function_name in 'generic' 'triangle'; do
      echo "Div function name: ${div_function_name}"
      { for _ in $(seq ${iter_count}); do
          './global_div' "${grid}" "${lvl_count}" "${div_function_name}" || exit 1
        done } | awk '/Operation time, ms: / {sum+=$4; count+=1}END{print "Total runs:", count, "\nAverage time:", sum/count, "ms"}' || exit 1
      echo "------------------------------"
    done
  fi
done

cd ..

