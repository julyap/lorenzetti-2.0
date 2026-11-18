# MuonCluster

## Visão Geral

Este diretório contém o módulo **MuonCluster**, responsável pela definição e
manipulação dos **clusters de múon** no EDM do ATLAS. Um cluster é formado
pelo agrupamento de células de múon (`MuonChamberCell`) em torno de uma
semente (`seed`), e carrega informações de energia, qualidade da track
(χ², q/p significance, ρ') e flags de working point.

O módulo fornece:

- A classe **`MuonCluster`** (herdada de `Gaugi::EDM`), que armazena as
  propriedades do cluster e um container de ponteiros para as células que o
  compõem;
- Os typedefs **`MuonClusterContainer`** para uso com o StoreGate;
- A struct POD **`MuonCluster_t`** e o **`MuonClusterConverter`**, usados
  para persistência em arquivos ROOT;
- O **LinkDef.h** para geração de dicionário ROOT;
- O **CMakeLists.txt** para build do módulo.

Os clusters são produzidos pelo `MuonClusterBuilder` (a partir das células
geradas pelo `MuonCellBuilder`) e consumidos pelo `MuonBuilder` para a
reconstrução final dos múons.

## Estrutura do Diretório

```
MuonCluster/
├── MuonCluster/                       # Headers públicos
│   ├── MuonCluster.h                  # Classe MuonCluster
│   ├── MuonClusterContainer.h         # Typedef SG::DataVector
│   └── MuonClusterConverter.h         # Struct MuonCluster_t + conversor
├── python/
│   └── __init__.py                    # Marcador de pacote Python
├── src/                               # Implementação C++
│   ├── MuonCluster.cxx                # Implementação de MuonCluster
│   ├── MuonClusterConverter.cxx       # Conversor
│   └── LinkDef.h                      # Definições ROOT
├── CMakeLists.txt                     # Configuração de build
└── README.md                          # Este arquivo
```

## Componentes

### `MuonCluster` (`MuonCluster.h` / `MuonCluster.cxx`)

Representa um cluster de múon com:

- **Propriedades básicas**: `emaxs2` (energia da célula mais energética),
  `eta`, `phi`, `deta`, `dphi`, `seed` (célula semente)
- **Energia**: `totalEnergy`, `fracMax`
- **Qualidade da track**: `chi2`, `qOverPSignificance`, `rhoPrime`
- **Flags de working point**: `isVeryLoose`, `isLoose`, `isMedium`,
  `isTight`, `quality` (código inteiro)
- **Variáveis de tracking**: `charge`, `pt`, `p`, `d0`, `z0`, `trackChi2`
- **Número de camadas com hits**: `nLayers`
- **Container de células**: `std::vector<const xAOD::MuonChamberCell*>`

## Dependências

- **GaugiKernel**: Núcleo do framework (`EDM`, `DataVector`, `macros`)
- **MuonCell**: Células de múon (`MuonChamberCell`, `MuonChamberConverter`)
- **EventInfo**: `SeedConverter` (para links com seeds)
- **ROOT** (`TTree`, `#pragma link`): Persistência e dicionário

