//
// Created by jyh_2 on 2021/6/18.
//

#ifndef QTREE_QUERYRANGE_H
#define QTREE_QUERYRANGE_H

#include "../common.h"

static int rangeCount;

class QueryRange{
public:
    int lower; // lower bound of the region
    int upper;  // upper bound of the region
    char boundInclude = 0; //0: neither include; 1 :lower include; 2: upper include; 3: both include

    QueryRange(){

        int r, diff;

        switch (dataRegionType){
            case Same:
                lower = rangeCount;
                upper = rangeCount + 1;
                break;
            case Random:
                r = (int) (rand() % maxValue);
                diff = (int) (rand() % span);
                lower = r ;
                upper = r + diff ;
                break;
//            case Zipf:
//                rand = Math.toIntExact(Tools.zipfianGenerator.nextValue());
//                diff =  Math.toIntExact(Tools.spanGenerator.nextValue());
//                lower = rand ;
//                upper = rand + diff ;
//                break;
            case Increase:
                lower = rangeCount ++ ;
                upper = rangeCount ++ ;
                break;
        }
        boundInclude = 3;
    }

    QueryRange( int startKey, int endKey, bool lowerInclude, bool upperInclude){
        lower = startKey;
        upper = endKey;
        if(lowerInclude && upperInclude){
            boundInclude = 3;
        }else if (lowerInclude) {
            boundInclude = 1;
        }else if (upperInclude){
            boundInclude = 2;
        }else{
            boundInclude = 0;
        }
    }

//    KeyHolder getLowerHolder(){
//        return new IntHolder(lower);
//    }
//
//    KeyHolder getUpperHolder(){
//        return new IntHolder(upper);
//    }

    bool cover(int value){
//        System.out.println("target:" + value + ", this:" + this);
        return  lower<= value && upper >= value;
    }

    friend ostream& operator<< (ostream &out, QueryRange* queryRange);

    bool operator< (QueryRange queryRange);

    bool operator<= (QueryRange queryRange);

    bool operator> (QueryRange queryRange);

    bool operator>= (QueryRange queryRange);

    bool MaxGE(QueryRange* queryRange);
    bool MinGT(QueryRange* queryRange);


};

#endif //QTREE_QUERYRANGE_H
