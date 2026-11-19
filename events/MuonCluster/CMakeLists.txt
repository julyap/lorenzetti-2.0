file(GLOB SOURCES src/*.cxx )
file(GLOB_RECURSE HEADERS MuonCluster/*.h)

include_directories(
    ${CMAKE_SOURCE_DIR} 
    ${ROOT_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/MuonCluster  # Adiciona o subdiretório
    ${CMAKE_CURRENT_SOURCE_DIR}/../../events/MuonCell # Adiciona MuonCell
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/GaugiKernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../../events/MuonDefs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../events/EventInfo # Adiciona EventInfo
    ${CMAKE_CURRENT_SOURCE_DIR}/../../reconstruction/MuonChamber/MuonClusterBuilder
)

ROOT_GENERATE_DICTIONARY(
    MuonClusterDict
    ${HEADERS}
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/LinkDef.h
    MODULE MuonCluster
)

add_library(MuonCluster OBJECT ${SOURCES} MuonClusterDict.cxx)
install(FILES ${HEADERS} DESTINATION MuonCluster)
gaugi_install_python_modules(${CMAKE_CURRENT_SOURCE_DIR}/python MuonCluster)

