file(GLOB SOURCES src/*.cxx )
file(GLOB_RECURSE HEADERS MuonCell/*.h )

include_directories(
    ${CMAKE_SOURCE_DIR} 
    ${ROOT_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/MuonCell  # Adiciona o subdiretório
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/GaugiKernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../../events/MuonDefs 
)
ROOT_GENERATE_DICTIONARY(MuonChamberDict ${HEADERS} LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/LinkDef.h  MODULE MuonChamber)
                                      
add_library(MuonCell OBJECT ${SOURCES} MuonChamberDict.cxx)
install(FILES ${HEADERS}  DESTINATION MuonChamber)

gaugi_install_python_modules( ${CMAKE_CURRENT_SOURCE_DIR}/python MuonChamber)
