cmake .
make
result=$1
n=20000000
<< 'COMMENT'
rm config.cfg
echo "TOTAL = $n
      TRACE_LEN = 1000000000
      dataRegionType = 1
      valueSpan = 20
      searchKeyType = 0" >> config.cfg

./QTree >> $result

rm config.cfg
echo "TOTAL = $n
      TRACE_LEN = 1000000000
      dataRegionType = 1
      valueSpan = 20
      searchKeyType = 1" >> config.cfg

./QTree >> $result

rm config.cfg
echo "TOTAL = $n
      TRACE_LEN = 2000000000
      dataRegionType = 1
      valueSpan = 16
      searchKeyType = 2" >> config.cfg

./QTree >> $result
COMMENT
rm config.cfg
echo "TOTAL = $n
      TRACE_LEN = 2000000000
      dataRegionType = 3
      valueSpan = 17
      searchKeyType = 0" >> config.cfg

./QTree >> $result

rm config.cfg
echo "TOTAL = $n
      TRACE_LEN = 2000000000
      dataRegionType = 3
      valueSpan = 17
      searchKeyType = 1" >> config.cfg

./QTree >> $result

rm config.cfg
echo "TOTAL = $n
      TRACE_LEN = 2000000000
      dataRegionType = 3
      valueSpan = 17
      searchKeyType = 2" >> config.cfg

./QTree >> $result