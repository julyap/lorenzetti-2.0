
cmake_minimum_required(VERSION 3.0)
project(lorenzetti)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DDROP_CGAL")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#----------------------------------------------------------------------------
# Set by hand pythia and fastjet
set(FASTJET_INCLUDE_DIRS $ENV{FASTJET_INCLUDE})
set(PYTHIA8_INCLUDE_DIRS $ENV{PYTHIA8_INCLUDE})
set(HEPMC_INCLUDE_DIRS $ENV{HEPMC_INCLUDE})

file(GLOB FASTJET_LIBRARIES
    $ENV{FASTJET_LIBRARIES}/*.dylib
    $ENV{FASTJET_LIBRARIES}/*.so
)

file(GLOB PYTHIA8_LIBRARIES
    $ENV{PYTHIA8_LIBRARIES}/*.a
    $ENV{PYTHIA8_LIBRARIES}/*.so
)

#----------------------------------------------------------------------------
# OpenMP
find_package(OpenMP)
if(OpenMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

#----------------------------------------------------------------------------
# Find Geant4
option(WITH_GEANT4_UIVIS
       "Build example with Geant4 UI and Vis drivers" ON)

if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Optional GeoModel
set(GEOMODEL_DIR $ENV{GEOMODEL_DIR}
    CACHE PATH "Install prefix of the GeoModel libraries")

set(GEOMODEL_LIBRARIES "")

if(GEOMODEL_DIR AND EXISTS "${GEOMODEL_DIR}/lib/libGeoModel2G4.so")

  message(STATUS "GeoModel found in ${GEOMODEL_DIR}")

  add_definitions(-DLORENZETTI_WITH_GEOMODEL)

  foreach(lib
      GeoModelRead
      GeoModelDBManager
      GeoModelKernel
      GeoModel2G4
      GeoMaterial2G4
      GeoModelHelpers
      GeoGenericFunctions
      GeoModelIOHelpers
      TFPersistification
  )
    list(APPEND GEOMODEL_LIBRARIES
         ${GEOMODEL_DIR}/lib/lib${lib}.so)
  endforeach()

  find_library(SQLITE3_LIBRARY NAMES sqlite3 libsqlite3.so.0)
  list(APPEND GEOMODEL_LIBRARIES ${SQLITE3_LIBRARY})

else()
  message(STATUS "GeoModel not found: --atlas-run4 disabled")
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${Geant4_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

#----------------------------------------------------------------------------
# Find ROOT
find_package(ROOT COMPONENTS
    EG Eve Geom Gui GuiHtml GenVector Hist Physics
    Matrix Graf RIO Tree Gpad RGL MathCore
)

include(${ROOT_USE_FILE})

#----------------------------------------------------------------------------
# Gaugi initialization
include(core/GaugiKernel/cmake/InstallGaugiFunctions.cmake)
gaugi_initialize()

#----------------------------------------------------------------------------
# Python
find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)

#----------------------------------------------------------------------------
# Simulation tools
include_directories(
    ${FASTJET_INCLUDE_DIRS}
    ${PYTHIA8_INCLUDE_DIRS}
    ${HEPMC_INCLUDE_DIRS}
)

#----------------------------------------------------------------------------
# Core
add_subdirectory(core/GaugiKernel)
add_subdirectory(core/G4Kernel)

#----------------------------------------------------------------------------
# Event packages
add_subdirectory(events/CaloHit)
add_subdirectory(events/CaloCell)
add_subdirectory(events/SpacePoint)
add_subdirectory(events/CaloCluster)
add_subdirectory(events/EventInfo)
add_subdirectory(events/TruthParticle)
add_subdirectory(events/CaloRings)
add_subdirectory(events/Egamma)

#----------------------------------------------------------------------------
# Muon event packages
add_subdirectory(events/Muon)
add_subdirectory(events/MuonHit)
add_subdirectory(events/MuonCluster)
add_subdirectory(events/MuonCell)

#----------------------------------------------------------------------------
# Detector definitions
add_subdirectory(geometry)

#----------------------------------------------------------------------------
# Calorimeter reconstruction
add_subdirectory(reconstruction/calorimeter/CaloCellBuilder)
add_subdirectory(reconstruction/calorimeter/CaloClusterBuilder)
add_subdirectory(reconstruction/calorimeter/CaloRingsBuilder)
add_subdirectory(reconstruction/calorimeter/CaloHitBuilder)

#----------------------------------------------------------------------------
# IO
add_subdirectory(reconstruction/io/RootStreamBuilder)

#----------------------------------------------------------------------------
# Physics reconstruction
add_subdirectory(reconstruction/physics/EgammaBuilder)

# Muon reconstruction
add_subdirectory(reconstruction/physics/MuonBuilder)

# Muon chamber reconstruction
add_subdirectory(reconstruction/MuonChamber/MuonCellBuilder)
add_subdirectory(reconstruction/MuonChamber/MuonClusterBuilder)

#----------------------------------------------------------------------------
# Reconstruction tools
add_subdirectory(reconstruction/reco)

#----------------------------------------------------------------------------
# Generator packages
add_subdirectory(generator/genkernel)
add_subdirectory(generator/filters)
add_subdirectory(generator/evtgen)

#----------------------------------------------------------------------------
# Main library
add_library(lorenzetti SHARED

  $<TARGET_OBJECTS:GaugiKernel>
  $<TARGET_OBJECTS:G4Kernel>

  # Calorimeter events
  $<TARGET_OBJECTS:CaloHit>
  $<TARGET_OBJECTS:CaloCell>
  $<TARGET_OBJECTS:CaloCluster>
  $<TARGET_OBJECTS:EventInfo>
  $<TARGET_OBJECTS:TruthParticle>
  $<TARGET_OBJECTS:CaloRings>
  $<TARGET_OBJECTS:Egamma>
  $<TARGET_OBJECTS:SpacePoint>

  # Muon events
  $<TARGET_OBJECTS:Muon>
  $<TARGET_OBJECTS:MuonHit>
  $<TARGET_OBJECTS:MuonCluster>
  $<TARGET_OBJECTS:MuonCell>

  # Detector geometry
  $<TARGET_OBJECTS:geometry>

  # Calorimeter reconstruction
  $<TARGET_OBJECTS:CaloHitBuilder>
  $<TARGET_OBJECTS:CaloCellBuilder>
  $<TARGET_OBJECTS:CaloClusterBuilder>
  $<TARGET_OBJECTS:CaloRingsBuilder>

  # IO
  $<TARGET_OBJECTS:RootStreamBuilder>

  # Physics reconstruction
  $<TARGET_OBJECTS:EgammaBuilder>
  $<TARGET_OBJECTS:MuonBuilder>

  # Muon chamber reconstruction
  $<TARGET_OBJECTS:MuonCellBuilder>
  $<TARGET_OBJECTS:MuonClusterBuilder>

  # Generators
  $<TARGET_OBJECTS:GenKernel>
  $<TARGET_OBJECTS:filters>
  $<TARGET_OBJECTS:evtgen>
)

#----------------------------------------------------------------------------
# Link libraries
target_link_libraries(lorenzetti
    ${ROOT_LIBRARIES}
    ${ROOT_COMPONENT_LIBRARIES}
    ${PYTHON_LIBRARIES}
    ${Geant4_LIBRARIES}
    ${FASTJET_LIBRARIES}
    ${HEPMC_LIBRARIES}
    ${PYTHIA8_LIBRARIES}
    ${GEOMODEL_LIBRARIES}
)

if(GEOMODEL_LIBRARIES)
  set_target_properties(lorenzetti PROPERTIES
      BUILD_RPATH "${GEOMODEL_DIR}/lib"
      INSTALL_RPATH "${GEOMODEL_DIR}/lib"
  )
endif()

#----------------------------------------------------------------------------
# Library information
message(STATUS "FASTJET_LIBRARIES: ${FASTJET_LIBRARIES}")
message(STATUS "PYTHIA8_LIBRARIES: ${PYTHIA8_LIBRARIES}")
message(STATUS "HEPMC_LIBRARIES: ${HEPMC_LIBRARIES}")
message(STATUS "GEOMODEL_LIBRARIES: ${GEOMODEL_LIBRARIES}")

