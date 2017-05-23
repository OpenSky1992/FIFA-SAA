PROJECT(SAA)
SET(SRC_LIST AllNextHop.cpp  TestCorrect.cpp Fib.cpp  Performance.cpp  common.cpp  Fib_update.cpp  Rib.cpp  UpdateTravel.cpp fifa.cpp )
ADD_DEFINITIONS(-std=c++11)
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
ADD_EXECUTABLE(saa ${SRC_LIST})

