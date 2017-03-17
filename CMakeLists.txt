PROJECT(SAA)
SET(SRC_LIST AllNextHop.cpp  Fib_bitmap.cpp	TestCorrect.cpp		AllNextHop.h    Fib.cpp         Performance.cpp  TestCorrect.h	CMakeLists.txt  Fib.h           Performance.h    TestModule.h	common.cpp      Fib_update.cpp  Rib.cpp          UpdateTravel.cpp	common.h	fifa.cpp        Rib.h            UpdateTravel.h)
ADD_DEFINITIONS(-std=c++11)
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
ADD_EXECUTABLE(saa ${SRC_LIST})

