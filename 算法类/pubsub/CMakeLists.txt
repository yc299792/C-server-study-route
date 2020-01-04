#NodeServer cmake config file for src

#cmake version request
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#project name
PROJECT(pubsub)

#Head file path
#INCLUDE_DIRECTORIES(../third/include)

#library path
#LINK_DIRECTORIES(../third/lib/jsoncpp)

#debug mode
SET(CMAKE_BUILD_TYPE Debug)

#################################################################
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR} SOURCE_FILES)
ADD_EXECUTABLE(pubsub ${SOURCE_FILES})

#for backtrace_symbols
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -m64 -pipe -std=c++11 -lrt -Wno-reorder")
#ADD_LIBRARY(nodeservice STATIC ${SOURCE_FILES})
#TARGET_LINK_LIBRARIES(pubsub thrift)
#zookeeper有st和mt两个库，需使用mt库
TARGET_LINK_LIBRARIES(pubsub pthread)
TARGET_LINK_LIBRARIES(pubsub libjsoncpp.a)


#install
#SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
INSTALL(TARGETS pubsub DESTINATION bin)