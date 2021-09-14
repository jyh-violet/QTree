cmake .
make
result=$1
n=20000000
tracelen=2000000000
dataRegionType=1
valueSpan=24
keytype=(0 1 2)
ratios=(0.5 0.75 0.95)
repeteNum=1

for r in ${ratios[*]}
do
  for type in ${keytype[*]}
  do
  rm config.cfg
  echo "TOTAL = $n
TRACE_LEN = $tracelen
dataRegionType = $dataRegionType
valueSpan = $valueSpan
searchKeyType = $type
insertRatio = $r" >> config.cfg
  echo "TOTAL = $n ,TRACE_LEN = $tracelen, dataRegionType = $dataRegionType, searchKeyType = $type, insertRatio=$r"  >> $result
  for i in `seq 1 $repeteNum`
  do
     ./QTree >> $result
  done
  done

done

dataRegionType=3

for r in ${ratios[*]}
do
  for type in ${keytype[*]}
  do
  rm config.cfg
  echo "TOTAL = $n
TRACE_LEN = $tracelen
dataRegionType = $dataRegionType
valueSpan = $valueSpan
searchKeyType = $type
insertRatio = $r" >> config.cfg
  echo "TOTAL = $n ,TRACE_LEN = $tracelen, dataRegionType = $dataRegionType, searchKeyType = $type, insertRatio=$r"  >> $result
  for i in `seq 1 $repeteNum`
  do
    ./QTree >> $result
  done
  done

done
