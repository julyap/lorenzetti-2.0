
file(GLOB SOURCES src/*.cxx)
file(GLOB_RECURSE HEADERS src/MuonMaker.h )
#file(GLOB_RECURSE HEADERS src/PhotonMaker.h )

include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../events/Muon)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonCluster)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonCell)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../events/EventInfo)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../core/GaugiKernel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../core/G4Kernel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../geometry)



ROOT_GENERATE_DICTIONARY(MuonBuilderDict ${HEADERS} LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/LinkDef.h  MODULE MuonBuilder)
                                      
add_library(MuonBuilder  OBJECT ${SOURCES} MuonBuilderDict.cxx)
install(FILES ${HEADERS}  DESTINATION MuonBuilder)


gaugi_install_python_modules( ${CMAKE_CURRENT_SOURCE_DIR}/python MuonBuilder)
