file(GLOB SOURCES src/*.cxx)
file(GLOB_RECURSE HEADERS src/*.h)

# remover LinkDef do dicionário
list(FILTER HEADERS EXCLUDE REGEX "LinkDef.h$")

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${ROOT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/TruthParticle
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonCell
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonHit
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../events/EventInfo
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../core/GaugiKernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../core/G4Kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../geometry

)

ROOT_GENERATE_DICTIONARY(
    MuonCellBuilderDict
    ${HEADERS}
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/LinkDef.h
    MODULE MuonCellBuilder
)

add_library(MuonCellBuilder OBJECT
    ${SOURCES}
    MuonCellBuilderDict.cxx
)

target_include_directories(MuonCellBuilder
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/MuonCellBuilder>
)

target_link_libraries(MuonCellBuilder
    PUBLIC
    GaugiKernel
    MuonHit
    MuonCell
    MuonHit
)

install(DIRECTORY src/
        DESTINATION include/MuonCellBuilder
        FILES_MATCHING PATTERN "*.h")

