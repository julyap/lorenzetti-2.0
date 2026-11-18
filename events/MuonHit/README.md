# MuonHit

## Visão Geral

Este diretório contém o módulo **MuonHit**, responsável pela definição e
manipulação dos **hits** do Espectrômetro de Múons inspirado no ATLAS. Um hit representa
a deposição de energia de uma partícula em uma célula da câmara de múon,
incluindo a evolução temporal do sinal ao longo dos Bunch Crossings (BCIDs).

O módulo fornece:

- A classe **`MuonHit`** (herdada de `Gaugi::EDM`), que armazena posição,
  identificação, energia, tempo e os mapas por BCID (`edep`, `tof`,
  `driftTime`);
- A struct POD **`MuonHit_t`** e o **`MuonHitConverter`**, usados para
  persistência em arquivos ROOT;
- Os typedefs **`MuonHitCollection`** e **`MuonHitContainer`** para uso com o
  StoreGate;
- O **LinkDef.h** para geração de dicionário ROOT.

Os hits são produzidos durante a simulação (via `fill()` a partir de um
`G4Step`) e consumidos pelo `MuonCellBuilder` para a formação de células.

## Estrutura do Diretório

```
MuonHit/
├── MuonHit/                     # Headers públicos
│   ├── MuonHit.h                # Classe MuonHit
│   ├── MuonHitCollection.h      # Typedef SG::Collection
│   ├── MuonHitContainer.h       # Typedef SG::DataVector
│   └── MuonHitConverter.h       # Struct MuonHit_t + conversor
├── src/                         # Implementação C++
│   ├── MuonHit.cxx              # Implementação de MuonHit
│   ├── MuonHitConverter.cxx     # Conversor
│   └── LinkDef.h                # Definições ROOT
├── CMakeLists.txt               # Configuração de build
└── README.md                    # Este arquivo
```

## Componentes

### `MuonHit` (`MuonHit.h` / `MuonHit.cxx`)

Representa um hit de múon com:

- **Posição e dimensões**: `x`, `y`, `z`, `dx`, `dy`
- **Identificação**: `hash`, `sampling`, `detector`, `region`
- **Configuração BCID**: `bcid_start`, `bcid_end`, `bc_duration`
- **Energia**: `energyDeposit` (acumulador rápido) e mapas por BCID
- **Sinal**: `time`, `pulse`
- **Flag**: `firstHit`


**Tratamento por detector:**

- **RPC**: registra o `tof` do primeiro hit com energia > 0
- **MDT**: `tof` = tempo; `driftTime` = distância radial / drift velocity (0.05 mm/ns)
- **TGC**: apenas `tof`



## Dependências

- **GaugiKernel**: Núcleo do framework (`EDM`, `macros`, `Collection`, `DataVector`)
- **MuonDefs**: Enumerações (`MuonSampling`, `MuonChamberType`, `MuonRegion`)
- **Geant4** (`G4Step`, `globals.hh`, `G4PhysicalConstants`, `G4SystemOfUnits`):
  Para preenchimento a partir dos steps de simulação
- **ROOT** (`TTree`, `#pragma link`): Persistência e dicionário
