#cmake .
#make clean
#make
result=$1
n=20000000
tracelen=2000000000
valueSpan=24
keytype=(0 2)
ratios=(0.5 0.75 0.95 1.0)
removePoint=(0)
dataRegionType=(1 3)
dataPointType=(0 2)
repeteNum=5
optimizationType=(0 1 2 3 )
checkQueryMeta=(1)
threadNum=(1 2 3 4 5 6 7 8)
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
              let t=t-1
	            echo -e "\n" >> $result
              for i in `seq 1 $repeteNum`
                do
            #     sudo /opt/intel/oneapi/vtune/2021.7.0/bin64/vtune -collect hotspots -result-dir=opt${opt}-d${data}-${r}-t${t}-hot  numactl --physcpubind=0-${t} ./QTree
            #     sudo /opt/intel/oneapi/vtune/2021.7.0/bin64/vtune -collect memory-access -result-dir=opt${opt}-d${data}-${r}-t${t}-mem  numactl --physcpubind=0-${t} ./QTree
                   numactl --physcpubind=0-${t} ./QTree >> $result
                done
              let t=t+1
            done
          done
        done
      done
    done
  done
done
done
