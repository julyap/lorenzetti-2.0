# Muon Chamber Integration in RootStreamBuilder

## Visão Geral

Este documento descreve todas as modificações realizadas nas classes do
diretório **RootStreamBuilder** para adicionar suporte completo à câmara de
múons (Muon Chamber) no pipeline de persistência e leitura de eventos
(HIT → ESD → AOD → Ntuple).

O objetivo dessas mudanças é permitir que os dados de múons (hits, células,
clusters e múons reconstruídos) sejam **escritos** e **lidos** de forma
consistente em todos os formatos do fluxo (HIT, ESD, AOD e Ntuple), mantendo
retrocompatibilidade com o calorímetro (Calo).

## Arquivos Modificados

### 1. `RootStreamKeys.py` — Novas chaves válidas

Foram adicionadas novas chaves ao conjunto `valid_keys` para permitir o
registro de coleções de múons:

- **Hits**: `MuonHits`, `MuonHits_Processed`, `TruthMuonHits`
- **Células**: `MuonCells`, `TruthMuonCells`
- **Clusters**: `MuonClusters`
- **Reconstruídos**: `Muons`

A função `recordable(key)` continua validando se a chave pertence a esse
conjunto, garantindo que nenhuma chave inválida seja registrada.

### 2. `RootStreamFlags.py` — Sem alterações

As flags de configuração (`RootStreamHITFlags`, `RootStreamESDFlags`,
`RootStreamAODFlags`) foram mantidas, mas o `RootStreamAODFlags` passou a
incluir `DumpCells = True` como default, e novas flags (`DumpMuonCells`,
`DumpMuonClusters`) foram adicionadas diretamente nos wrappers Python.

### 3. `RootStreamHITMaker.py/.cxx/.h` — Hits de múons

**Mudanças no Python (`RootStreamHITMaker.py`)**:
- Novos parâmetros `InputMuonHitsKey` e `OutputMuonHitsKey` (default `"MuonHits"`).
- Novos parâmetros `MuonEtaWindow` e `MuonPhiWindow` para o filtro RoI de múons.
- Novo parâmetro `EnableMuonOutput` (default `True`).
- Defaults expandidos com `InputCaloHitsKey`, `InputMuonHitsKey` e janelas de múon.
- Fallback para criação da instância C++ (tenta `ROOT.`, `ROOT.SG.`, `ROOT.Gaugi.`).

**Mudanças no C++ (`RootStreamHITMaker.cxx/.h`)**:
- Novas propriedades declaradas: `InputMuonHitsKey`, `OutputMuonHitsKey`,
  `MuonEtaWindow`, `MuonPhiWindow`, `EnableMuonOutput`.
- `bookHistograms` agora cria dois branches de hits: `CaloHitContainer_<key>`
  e `MuonHitContainer_<key>`.
- `serialize` agora processa:
  - `CaloHits` (como antes, com filtro RoI usando partículas de calo);
  - `MuonHits` (novo), com filtro RoI usando partículas de múon.
- Funções auxiliares `isMuon(pdgId)`, `calculateEta`, `calculatePhi` foram
  adicionadas para identificar múons e calcular η/φ a partir das coordenadas
  espaciais.
- Logs detalhados do número de hits de múon e energia total depositada.

### 4. `RootStreamHITReader.py/.cxx/.h` — Leitura de hits de múons

**Mudanças no Python (`RootStreamHITReader.py`)**:
- `OutputCaloHitsKey` (renomeado de `OutputHitsKey`).
- Novo `OutputMuonHitsKey` (default `"MuonHits"`).
- Novo `EnableMuonInput` (default `False`) para controlar se os hits de múon
  serão lidos.

**Mudanças no C++ (`RootStreamHITReader.cxx/.h`)**:
- Novas propriedades: `OutputMuonHitsKey`, `EnableMuonInput`.
- `deserialize` agora inicializa dois branches de hits:
  - `CaloHitContainer_CaloHits` → `xAOD::CaloHitContainer`;
  - `MuonHitContainer_MuonHits` → `xAOD::MuonHitContainer`.
- Conversão de `MuonHit_t` → `xAOD::MuonHit` via `MuonHitConverter`.
- Logs do número de CaloHits e MuonHits convertidos por evento.

### 5. `RootStreamESDMaker.py/.cxx/.h` — Células de múons (ESD)

**Mudanças no Python (`RootStreamESDMaker.py`)**:
- Novos parâmetros de entrada: `InputMuonCellsKey`, `InputTruthMuonCellsKey`
  e `InputMuonCellKeys` (lista, para combinar múltiplas coleções).
- Novos parâmetros de saída: `OutputMuonCellsKey` (default `"MuonCells"`).
- Novas flags: `EnableCaloOutput` e `EnableMuonOutput`.
- Conversão da lista Python `InputMuonCellKeys` para `ROOT.std.vector('std::string')`.

**Mudanças no C++ (`RootStreamESDMaker.cxx/.h`)**:
- Novas propriedades: `InputMuonCellsKey`, `InputTruthMuonCellsKey`,
  `InputMuonCellKeys`, `OutputMuonCellsKey`, `EnableCaloOutput`,
  `EnableMuonOutput`.
- `initialize` auto-habilita `EnableMuonOutput` se qualquer chave de múon
  for fornecida.
- `bookHistograms` agora cria branches separados para:
  - calo: `CaloCellContainer_<key>`, `CaloDetDescriptorContainer_<key>`;
  - múon: `MuonChamberCellContainer_<key>`, `MuonChamberDescriptorContainer_<key>`.
- `serialize` foi dividido em duas seções:
  - **Calorímetro**: mantém a lógica original (filtro RoI por partículas de verdade).
  - **Múon**: novo bloco com prioridade para a lista de chaves
    (`m_inputMuonCellKeys`), com fallback para a chave única
    (`m_inputMuonCellsKey`). Para cada célula, converte via
    `MuonChamberConverter` e preenche explicitamente `sampling`, `detector`,
    `chamberType`, `region`, `layer` a partir do descriptor.
  - Se o container de células não for encontrado, tenta ler o
    `MuonChamberDetDescriptorCollection` diretamente, criando células
    sintéticas com η/φ calculados a partir de x, y, z.
- Novo campo nas `TruthParticle_t` copiando variáveis do par (`pair_pdg1`,
  `pair_mass`, `pair_delta_R`, etc.).
- Uso de `std::set<unsigned long> processed_descriptors` para evitar
  duplicação de células entre múltiplas coleções.

### 6. `RootStreamESDReader.py/.cxx/.h` — Leitura de células de múons

**Mudanças no Python (`RootStreamESDReader.py`)**:
- Novo parâmetro `OutputMuonCellsKey` (default `"MuonCells"`).
- Novos parâmetros `EnableCaloOutput`, `EnableMuonOutput` e `MuonCellKeys`
  (lista).
- Se `MuonCellKeys` for fornecida, força `EnableMuonOutput = True`.
- Log de configuração detalhado, incluindo cada chave de múon.

**Mudanças no C++ (`RootStreamESDReader.cxx/.h`)**:
- Novas propriedades: `OutputMuonCellsKey`, `EnableCaloOutput`,
  `EnableMuonOutput`, `MuonCellKeys`.
- `deserialize` lê dois modos de branches de múon:
  - **Single key**: `MuonChamberCellContainer_<key>` e
    `MuonChamberDetDescriptorContainer_<key>`;
  - **Multiple keys**: para cada chave em `MuonCellKeys`, abre um par de
    branches próprio e guarda em `std::map<std::string, std::vector<...>*>`.
- Reconstrução do hash do descriptor de múon a partir dos campos
  `detector`, `sampling`, `region`, `layer` (via bit-shift) para contornar
  a ausência do hash serializado.
- Criação manual do `MuonChamberDescriptor` no leitor quando necessário.
- Preenchimento dos campos `m_energy`, `m_eta`, `m_deltaEta`, `m_deltaPhi`,
  `m_layer` da célula reconstruída.

### 7. `RootStreamAODMaker.py/.cxx/.h` — AOD com múons

**Mudanças no Python (`RootStreamAODMaker.py`)**:
- Novos parâmetros de entrada: `InputMuonKey`, `InputMuonCellsKey`,
  `InputMuonClusterKey`.
- Novos parâmetros de saída: `OutputMuonKey`, `OutputMuonCellsKey`,
  `OutputMuonClusterKey`.
- Novas flags: `EnableCaloOutput`, `EnableMuonOutput`.
- Novas flags de dump: `DumpMuonCells`, `DumpMuonClusters` (default `False`).
- Conversão automática: se a chave de saída não for fornecida, usa a de entrada.

**Mudanças no C++ (`RootStreamAODMaker.cxx/.h`)**:
- Novas propriedades: todas as chaves de múon acima + `DumpMuonCells`,
  `DumpMuonClusters`.
- `bookHistograms` cria três novos branches de múon:
  - `MuonContainer_<key>` (múons reconstruídos);
  - `MuonChamberCellContainer_<key>` e
    `MuonChamberDetDescriptorContainer_<key>` (células);
  - `MuonClusterContainer_<key>` (clusters).
- `serialize` foi dividido em blocos por sistema:
  - **TruthParticle**: agora calcula e preenche as variáveis do par
    (`pair_mass`, `pair_delta_R`, etc.) agrupando por `seedid`.
  - **Calorímetro**: mantém lógica original.
  - **Múon**: novo bloco serializando `MuonChamberCell` →
    `MuonChamber_t`, `MuonChamberDescriptor` → `MuonChamberDescriptor_t`,
    `MuonCluster` → `MuonCluster_t`, e `Muon` → `Muon_t`.
- Logs coloridos distinguindo WARNING (container vazio) de INFO (serializado).

### 8. `RootStreamAODReader.py/.cxx/.h` — Leitura de AOD com múons

**Mudanças no Python (`RootStreamAODReader.py`)**:
- Novo parâmetro `OutputMuonKey` (default `"Muons"`).
- Novo parâmetro `OutputMuonClusterKey` (default `"MuonClusters"`).
- Novas flags `EnableCaloOutput`, `EnableMuonOutput`.
- Contagem de entradas via `TFile` + `TTree::GetEntries()`.
- Log detalhado da configuração (calo vs múon).

**Mudanças no C++ (`RootStreamAODReader.cxx/.h`)**:
- Novas propriedades: `OutputMuonKey`, `OutputMuonClusterKey`,
  `EnableCaloOutput`, `EnableMuonOutput`.
- `deserialize` inicializa branches condicionalmente:
  - Calo (se `m_enableCaloOutput`): clusters, rings, electrons, células.
  - Múon (se `m_enableMuonOutput`): `MuonContainer_<key>`,
    `MuonChamberCellContainer_<key>`,
    `MuonChamberDetDescriptorContainer_<key>`,
    `MuonClusterContainer_<key>`.
- Conversão de `MuonCluster_t` → `xAOD::MuonCluster` e
  `Muon_t` → `xAOD::Muon`.
- Nota: os muon cells são serializados/deserializados como `MuonChamber_t`
  (não como `MuonChamberCell` diretamente), exigindo um cuidado extra na
  reconstrução dos pointers.

### 9. `RootStreamNtupleMaker.py/.cxx/.h` — Ntuple com múons

**Mudanças no Python (`RootStreamNtupleMaker.py`)**:
- Novos parâmetros de entrada: `InputMuonKey`, `InputMuonClusterKey`.
- Novas flags `EnableCaloOutput`, `EnableMuonOutput`.
- Fallback de namespace: tenta `ROOT.Gaugi.` e depois `ROOT.` (sem namespace).

**Mudanças no C++ (`RootStreamNtupleMaker.cxx/.h`)**:
- Novas propriedades: `InputMuonKey`, `InputMuonClusterKey`,
  `InputCaloCellsKey`, `InputMuonCellsKey`, todas as chaves de saída
  correspondentes, e as flags `DumpCells`, `DumpMuonCells`, `DumpMuonClusters`.
- `bookHistograms` cria branches separados para:
  - calo: `CaloRingsContainer_<key>`, `CaloClusterContainer_<key>`,
    `ElectronContainer_<key>`, e (se `DumpCells`) `CaloCellContainer_<key>`
    + `CaloDetDescriptorContainer_<key>`;
  - múon: `MuonContainer_<key>`, `MuonClusterContainer_<key>`, e (se
    `DumpMuonCells`) `MuonChamberCellContainer_<key>` +
    `MuonChamberDetDescriptorContainer_<key>`.
- `serialize` reutiliza a mesma lógica do AODMaker para TruthParticle com
  variáveis de par, e adiciona blocos de múon análogos, controlados pelas
  flags de dump.

### 10. `LinkDef.h` — Dicionário ROOT

Foram adicionadas entradas para as classes do stream builder com e sem
namespace, garantindo que o ROOT gere o dicionário correto para PyROOT/CINT:

- `Gaugi::RootStreamHITMaker` e `RootStreamHITMaker`
- `Gaugi::RootStreamNtupleMaker` e `RootStreamNtupleMaker`
- `Gaugi::RootStreamHITReader`
- `RootStreamESDReader` e `Gaugi::RootStreamESDReader`
- Namespaces `SG`, `Gaugi`, `xAOD`
- Tipos serializáveis: `xAOD::Hit_t` e `std::vector<xAOD::Hit_t>`

### 11. `__init__.py` — Imports do pacote

O `__init__.py` foi atualizado para importar e reexportar todos os módulos
novos, incluindo `RootStreamFlags` (que antes não era reexportado),
mantendo o padrão:

```python
from . import RootStream<X>
__all__.extend(RootStream<X>.__all__)
from .RootStream<X> import *
