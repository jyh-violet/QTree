cmake .
make
n=20000000
tracelen=2000000000
valueSpan=24
keytype=(0)
ratios=( 1.0)
removePoint=(0)
dataRegionType=(1)
dataPointType=(0)
repeteNum=1
optimizationType=(0 1 )
checkQueryMeta=(1)
threadNum=(1 2 3 4)
for t in ${threadNum[*]}
do
for checkQ in ${checkQueryMeta[*]}
do
  for opt in ${optimizationType[*]}
  do
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
dataPointType = $dataPoint
optimizationType = $opt
checkQueryMeta = $checkQ
threadnum = $t" >> config.cfg
              for i in `seq 1 $repeteNum`
                do
        #         sudo /opt/intel/oneapi/vtune/2021.7.0/bin64/vtune -collect hotspots -result-dir=opt${opt}-d${data}-t${t}-hot  numactl --physcpubind=0-3 ./QTree
        #         sudo /opt/intel/oneapi/vtune/2021.7.0/bin64/vtune -collect memory-access -result-dir=opt${opt}-d${data}-t${t}-mem  numactl --physcpubind=0-3 ./QTree
                   numactl --physcpubind=0-3 ./QTree
                done
            done
          done
        done
      done
    done
  done
done
done