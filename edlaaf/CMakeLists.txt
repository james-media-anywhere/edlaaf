# CMakeList.txt : CMake project for anywhere, include source and define
# project specific logic here.
#

add_executable (MeaAAF   
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

target_include_directories(MeaAAF PUBLIC 
	${AAF_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
)

target_link_directories(MeaAAF PUBLIC 
	${AAF_LIB_DIR}
)

target_link_libraries(MeaAAF PUBLIC 
	${Boost_LIBRARIES}
	"aaf.lib"
	"AAFCOAPI.lib"
	"AAFIID.lib"
)
