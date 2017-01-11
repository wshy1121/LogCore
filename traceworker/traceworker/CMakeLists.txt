
PROJECT(HMSProxy)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fPIC -O0")

SET(SRC_SRCS
    trace_worker.cpp
    trace_base.cpp
    time_calc.cpp
    platform_base.cpp
    mem_calc.cpp
    log_opr.cpp
    locale_file.cpp
    link_tool.cpp
    IFile.cpp
    file_manager.cpp
)

ADD_LIBRARY(TraceWorker SHARED ${SRC_SRCS})
#ADD_LIBRARY(TraceWorker STATIC ${SRC_SRCS})

TARGET_LINK_LIBRARIES(TraceWorker)

SET_TARGET_PROPERTIES(TraceWorker PROPERTIES CLEAN_DIRECT_OUTPUT 1)


