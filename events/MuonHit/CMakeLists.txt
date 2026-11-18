# Coleta todos os arquivos fonte .cxx recursivamente
file(GLOB_RECURSE SOURCES src/*.cxx src/helper/*.cxx )

# events/MuonHit/CMakeLists.txt
file(GLOB_RECURSE HEADERS MuonHit/*.h MuonHit/helper/*.h MuonHit/MuonDefs.h)
# Configuração dos diretórios de inclusão
include_directories(
    ${CMAKE_SOURCE_DIR} 
    ${ROOT_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/MuonHit  # Adiciona o subdiretório
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/GaugiKernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../MuonCell     
    ${CMAKE_CURRENT_SOURCE_DIR}/../../events/MuonDefs         # Células de múons
)

# Geração do dicionário ROOT para reflexão e persistência
ROOT_GENERATE_DICTIONARY(
    MuonHitDict              # Nome do dicionário
    ${HEADERS}               # Headers para gerar dicionário
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/src/LinkDef.h  # Arquivo de definição de link
    MODULE MuonHit           # Nome do módulo
)

# Criação da biblioteca object (para linkagem com outros componentes)
add_library(MuonHit  
    OBJECT                  # Biblioteca do tipo object (não shared/static)
    ${SOURCES}             # Todos os arquivos fonte
    MuonHitDict.cxx        # Dicionário gerado pelo ROOT
)

# Instalação dos headers para uso por outros pacotes
install(
    FILES ${HEADERS}       # Todos os headers coletados
    DESTINATION MuonHit    # Diretório de instalação
)

# Instalação de módulos Python (se aplicável)
#gaugi_install_python_modules(${CMAKE_CURRENT_SOURCE_DIR}/python MuonHit)
