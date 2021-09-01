cmake_minimum_required(VERSION 3.16)
project(QTree)

set(CMAKE_CXX_STANDARD 14)


#3.head file path，头文件目录
INCLUDE_DIRECTORIES(
        src
)


#4.source directory，源文件目录
AUX_SOURCE_DIRECTORY(src DIR_SRCS)

#5.set environment variable，设置环境变量，编译用到的源文件全部都要放到这里，否则编译能够通过，但是执行的时候会出现各种问题，比如"symbol lookup error xxxxx , undefined symbol"
SET(TEST_MATH
        ${DIR_SRCS}
        src/index/QTree.cpp src/Tool/Tools.cpp)


add_library(query src/query/QueryRange.cpp src/query/QueryMeta.cpp src/query/QueryRange.h src/query/QueryMeta.h)
add_library(index src/index/Node.h src/index/Node.cpp src/index/QTree.h src/index/QTree.cpp  src/index/InternalNode.h src/index/LeafNode.h src/index/Node.cpp)
target_link_libraries(index PUBLIC query)

add_library(main src/main.cpp)
target_link_libraries(main PUBLIC index)

#6.add executable file，添加要编译的可执行文件
ADD_EXECUTABLE(${PROJECT_NAME}  ${TEST_MATH})


#7.add link library，添加可执行文件所需要的库，比如我们用到了libm.so（命名规则：lib+name+.so），就添加该库的名称
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} m)
target_link_libraries(${PROJECT_NAME}  PUBLIC main)



#add_executable(QTree main.cpp src/query/QueryTemplate.cpp include/query/QueryTemplate.h src/query/QueryMeta.cpp include/query/QueryMeta.h include/common.h include/query/Tools.h include/holder/RangeHolder.h include/index/QTree.h include/index/Node.h include/index/LeafNode.h include/index/InternalNode.h src/query/QueryRange.cpp include/query/QueryRange.h)