# MuonDefs

## Visão Geral

Este diretório contém o header **MuonDefs.h**, que reúne as definições
auxiliares para o subsistema de múons no EDM (Event Data Model). Ele fornece:

- A enumeração **`MuonSampling`** (Inner/Middle/Outer), que complementa as
  enumerações já existentes (`MuonChamberType`, `MuonRegion`, `MuonLayer`,
  `MuonStation`);
- Funções utilitárias de **conversão enum → string**;
- Funções de **conversão entre enumerações**;
- A struct **`MuonChamberId`**, que agrega os identificadores de uma câmara
  e fornece um método `toString()`.

Essas definições são usadas para **nomear, indexar e comparar** câmaras de
múon em toda a cadeia de simulação e reconstrução.

## Estrutura do Diretório

```
MuonDefs/
├── MuonDefs.h        # Header com enum, funções e struct
└── README.md         # Este arquivo
```


