cmake .
make
result=$1
n=20000000
tracelen=2000000000
valueSpan=24
keytype=(0 2 3)
ratios=(0.5 0.75 0.95 1.0)
removePoint=(0)
dataRegionType=(1 3)
dataPointType=(0 1 2)
repeteNum=5
for dataPoint in ${dataPointType[*]}
do
  for data in ${dataRegionType[*]}
  do
    for point in ${removePoint[*]}
    do
      for r in ${ratios[*]}
      do
        for type in ${keytype[*]}
        do
          rm config.cfg
          echo "TOTAL = $n
RACE_LEN = $tracelen
dataRegionType = $data
valueSpan = $valueSpan
searchKeyType = $type
insertRatio = $r
removePoint = $point
dataPointType = $dataPoint" >> config.cfg
                echo -e "\n" >> $result
          for i in `seq 1 $repeteNum`
            do
               ./QTree >> $result
            done
        done
      done
    done
  done
done