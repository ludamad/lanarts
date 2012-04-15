CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(RVO)

SET(RVO_SRCS Agent.h Agent.cpp Definitions.h KdTree.h KdTree.cpp Obstacle.h Obstacle.cpp RVO.h RVOSimulator.h RVOSimulator.cpp Vector2.h)

ADD_LIBRARY(RVO STATIC ${RVO_SRCS})

INSTALL(FILES RVO.h RVOSimulator.h Vector2.h DESTINATION ../include)
INSTALL(TARGETS RVO DESTINATION ../lib)
