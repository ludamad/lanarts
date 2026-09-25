# RVO2 2.0 collision avoidance, extended for lanarts with RVOSimulator::removeAgent/changeAgentNo
add_library(rvo2 STATIC Agent.cpp KdTree.cpp Obstacle.cpp RVOSimulator.cpp)
target_include_directories(rvo2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
