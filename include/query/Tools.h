//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_TOOLS_H
#define QTREE_TOOLS_H


enum DataRegionType {
    Same,
    Random,
    Increase,
    Zip
};

static const DataRegionType dataRegionType = Random;
static int valueSpan = 21; // 2 ^valueSpan
static int maxValue = 1 << (valueSpan - 1);
static int span = 1000;
static int redunc = 5;  // 2^redunc
static int coordianteRedunc = 2;  // 2^redunc
static int BOrder = 32;

#endif //QTREE_TOOLS_H
