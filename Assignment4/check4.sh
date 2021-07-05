#!/bin/bash

#######################################################################
#
# Copyright (C) 2020-2021 David C. Harrison. All right reserved.
#
# You may not use, distribute, publish, or modify this code without 
# the express written permission of the copyright holder.
#
#######################################################################

(( total = 0))

echo ""
> test.out
(( check = ( RANDOM % 1024 )  + 1 ))
p=0
while (( p <= 10 ))
do
  limit=$(( 2**p ))
  for f in {1..32}
  do
    (( total += 1))
    (( lower = limit ))
    if (( lower > 16 ))
    then
      dif=`shuf -i 2-60 -n 1`
      (( lower -= dif ))
    fi
    cnt=`shuf -i $lower-$limit -n 1`
    timeout 1s ./scheduler -t $cnt --fcfs -c $check >> test.out
    exit_status=$?
    if [[ $exit_status -eq 124 ]]; then
      echo "FAIL - Timeout" >> test.out
    fi
    echo -n "$cnt "
  done
  (( p += 2 ))
done
echo ""

pass=`grep "PASS $check" test.out | wc -l`

if [ "$pass" -lt "0" ]; then
  pass=0
fi

pct=0;
if (( pass > 0 ))
then
  pct=`echo "scale=2; $pass / $total * 90.0" | bc -l`
fi

printf "\n%20s:  %3d/%3d  %6.1f%% of 90%%\n" "Tests" $pass $total $pct
