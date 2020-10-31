#!/usr/bin/env sh

grid=src/icon_grid_0019_R02B05_G.nc
lvl_count=40
iter_count=50

for div_function_name in 'generic' 'triangle'; do
  echo "------------------------------"
  echo "Div function name: ${div_function_name}"
  { for _ in $(seq ${iter_count}); do
      './global_div' "${grid}" "${lvl_count}" "${div_function_name}" || exit 1
    done } | awk '/Operation time, ms: / {sum+=$4; count+=1}END{print "Total runs:", count, "\nAverage time:", sum/count, "ms"}' || exit 1
done
