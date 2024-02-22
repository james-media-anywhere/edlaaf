# CMakeList.txt : CMake project for anywhere, include source and define
# project specific logic here.
#

add_executable (edlaaf   
	cAafEdl.cpp
	cCut.cpp
	cDisolve.cpp
	cEdl.cpp
	cEdlData.cpp
	cEvent.cpp
	cLine.cpp
	createAaf.cpp
	cSegment.cpp
	cTimeCode.cpp
	cToken.cpp
	cTrack.cpp
	cWipe.cpp
	edlaaf.cpp
	StdAfx.cpp
)

target_include_directories(edlaaf PUBLIC 
	${AAF_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
)

target_link_directories(edlaaf PUBLIC 
	${AAF_LIB_DIR}
)

target_link_libraries(edlaaf PUBLIC 
	${Boost_LIBRARIES}
	"aaf.lib"
	"AAFCOAPI.lib"
	"AAFIID.lib"
)
