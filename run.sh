cmake .
make
result=$1
n=20000000
tracelen=2000000000
dataRegionType=1
valueSpan=24

<< 'COMMENT'
rm config.cfg
echo "TOTAL = $n
TRACE_LEN = $tracelen
dataRegionType = $dataRegionType
valueSpan = $valueSpan
searchKeyType = 0" >> config.cfg
echo "TOTAL = $n ,TRACE_LEN = $tracelen, dataRegionType = $dataRegionType, valueSpan = $valueSpan, searchKeyType = 0" >> $result
for i in `seq 1 10`
do
  ./QTree >> $result
done


rm config.cfg
echo "TOTAL = $n
TRACE_LEN = $tracelen
dataRegionType = $dataRegionType
valueSpan = $valueSpan
searchKeyType = 1" >> config.cfg
echo "TOTAL = $n ,TRACE_LEN = $tracelen, dataRegionType = $dataRegionType, valueSpan = $valueSpan, searchKeyType = 1" >> $result
for i in `seq 1 10`
do
  ./QTree >> $result
done


rm config.cfg
echo "TOTAL = $n
TRACE_LEN = $tracelen
dataRegionType = $dataRegionType
valueSpan = $valueSpan
searchKeyType = 2" >> config.cfg
echo "TOTAL = $n ,TRACE_LEN = $tracelen, dataRegionType = $dataRegionType, valueSpan = $valueSpan, searchKeyType = 2" >> $result
for i in `seq 1 10`
do
  ./QTree >> $result
done
COMMENT

dataRegionType=3

rm config.cfg
echo "TOTAL = $n
TRACE_LEN = $tracelen
dataRegionType = $dataRegionType
valueSpan = $valueSpan
searchKeyType = 0" >> config.cfg
echo "TOTAL = $n ,TRACE_LEN = $tracelen, dataRegionType = $dataRegionType, valueSpan = $valueSpan, searchKeyType = 0" >> $result
for i in `seq 1 10`
do
  ./QTree >> $result
done



rm config.cfg
echo "TOTAL = $n
TRACE_LEN = $tracelen
dataRegionType = $dataRegionType
valueSpan = $valueSpan
searchKeyType = 1" >> config.cfg
echo "TOTAL = $n ,TRACE_LEN = $tracelen, dataRegionType = $dataRegionType, valueSpan = $valueSpan, searchKeyType = 1" >> $result
for i in `seq 1 10`
do
  ./QTree >> $result
done


rm config.cfg
echo "TOTAL = $n
TRACE_LEN = $tracelen
dataRegionType = $dataRegionType
valueSpan = $valueSpan
searchKeyType = 2" >> config.cfg
echo "TOTAL = $n ,TRACE_LEN = $tracelen, dataRegionType = $dataRegionType, valueSpan = $valueSpan, searchKeyType = 2" >> $result
for i in `seq 1 10`
do
  ./QTree >> $result
done
