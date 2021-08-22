//
// Created by jyh_2 on 2021/6/18.
//

#include "../../include/query/QueryRange.h"

ostream& operator<< (ostream &out, QueryRange* queryRange){
    out << "{" << queryRange->lower << ", " << queryRange->upper << "}";
    return out;
}

bool QueryRange::operator< (QueryRange queryRange){
    return this->lower < queryRange.lower;
}

bool QueryRange::operator<= (QueryRange queryRange){
    return this->lower <= queryRange.lower;
}

bool QueryRange::operator> (QueryRange queryRange){
    return this->lower > queryRange.lower;
}

bool QueryRange::operator>= (QueryRange queryRange){
    return this->lower >= queryRange.lower;

}

bool QueryRange::MaxGE(QueryRange *queryRange){
    return this->upper >= queryRange->upper;
}

bool QueryRange::MinGT(QueryRange *queryRange){
    return this->lower > queryRange->lower;
}