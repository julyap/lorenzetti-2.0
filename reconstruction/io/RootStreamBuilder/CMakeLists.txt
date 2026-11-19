file(GLOB SOURCES src/*.cxx)

# CORREÇÃO: antes era apenas src/RootStreamMaker.h — o rootcling
# ignorava RootStreamHITReader.h, RootStreamHITMaker.h, etc.
file(GLOB_RECURSE HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)
list(FILTER HEADERS EXCLUDE REGEX ".*/LinkDef\\.h$")

# Inclui diretórios
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${ROOT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/Egamma
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/Muon
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/CaloCluster
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonCluster
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/CaloCell
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonCell
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/CaloHit
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonHit
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/EventInfo
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/TruthParticle
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/CaloRings
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../core/GaugiKernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../core/G4Kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events
)

ROOT_GENERATE_DICTIONARY(RootStreamBuilderDict ${HEADERS} LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/LinkDef.h  MODULE RootStreamBuilder)

add_library(RootStreamBuilder  OBJECT ${SOURCES} RootStreamBuilderDict.cxx)
install(FILES ${HEADERS}  DESTINATION RootStreamBuilder)

gaugi_install_python_modules( ${CMAKE_CURRENT_SOURCE_DIR}/python RootStreamBuilder)
