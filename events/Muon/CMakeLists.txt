# Coletar arquivos fonte
file(GLOB SOURCES src/*.cxx)

# Coletar arquivos headers (recursivamente na pasta Muon)
file(GLOB_RECURSE HEADERS Muon/*.h)

# Adicionar diretórios de include
include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../core/GaugiKernel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../core/G4Kernel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../MuonCell)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../MuonCluster)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../EventInfo)

# Gerar dicionário ROOT
ROOT_GENERATE_DICTIONARY(MuonDict ${HEADERS} LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/LinkDef.h MODULE Muon)

# Adicionar biblioteca
add_library(Muon OBJECT ${SOURCES} MuonDict.cxx)

# Instalar headers
install(FILES ${HEADERS} DESTINATION Muon)

# Instalar módulos Python
gaugi_install_python_modules(${CMAKE_CURRENT_SOURCE_DIR}/python Muon)
