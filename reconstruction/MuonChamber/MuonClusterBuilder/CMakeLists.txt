# Find ROOT
find_package(ROOT REQUIRED)

# Source files
file(GLOB SOURCES src/*.cxx )
file(GLOB_RECURSE HEADERS src/M*.h src/ShowerShapes.h )

include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonCluster)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonCell)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../events/EventInfo)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../events/TruthParticle)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../events/MuonChamberRings)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../core/GaugiKernel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../core/G4Kernel)

# Generate dictionary with UNIQUE name for muon cluster builder
ROOT_GENERATE_DICTIONARY(MuonClusterBuilderDict ${HEADERS} LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/LinkDef.h MODULE MuonClusterBuilder)

# Create library with UNIQUE name
add_library(MuonClusterBuilder OBJECT ${SOURCES} MuonClusterBuilderDict.cxx)

# Link dependencies
target_link_libraries(MuonClusterBuilder
    PUBLIC
    GaugiKernel
    MuonCluster
    ${ROOT_LIBRARIES}
)

# Install headers
install(FILES ${HEADERS} DESTINATION include/MuonClusterBuilder)

# Install python modules
gaugi_install_python_modules(${CMAKE_CURRENT_SOURCE_DIR}/python MuonClusterBuilder)
