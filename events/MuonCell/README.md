# MuonCell

## Visão Geral

Este diretório contém o módulo **MuonCell**, responsável pela definição e manipulação das células do Espectrômetro de Múons do ATLAS. Ele fornece:

- As **enumerações** (`MuonDefs.py`) que padronizam tipos de câmara, estações, camadas e regiões;
- As **classes de dados** que representam células (`MuonChamberCell`) e descriptors geométricos/físicos (`MuonChamberDescriptor`);
- Os **conversores** (`MuonChamberConverter` e
  `MuonChamberDescriptorConverter`) para serialização/desserialização em arquivos ROOT;
- O **LinkDef.h** para geração de dicionário ROOT.

Essas células são a unidade fundamental produzida pelo `MuonCellBuilder` e consumida pelo `MuonClusterBuilder` para a formação de clusters de múons.

## Estrutura do Diretório

```
MuonChamber/
├── MuonCell/                    # Módulo atual - Células de múon
│   ├── MuonDefs.py              # Enumerações (câmara, estação, camada, região)
│   ├── src/                     # Código fonte C++
│   │   ├── MuonCell.cxx         # Implementação de MuonChamberCell
│   │   ├── MuonChamberConverter.cxx           # Conversor de células
│   │   ├── MuonChamberDescriptorConverter.cxx # Conversor de descriptors
│   │   ├── MuonChamberDetDescriptor.cxx       # Implementação do descriptor
│   │   └── LinkDef.h            # Definições para dicionário ROOT
│   ├── CMakeLists.txt           # Configuração de build
│   └── README.md                # Este arquivo
```

## Componentes

### `MuonDefs.py`

Define as enumerações usadas em toda a cadeia de múons:

Essas enumerações garantem consistência entre os códigos Python e C++.

### `MuonChamberCell` (`MuonCell.cxx`)

Representa uma célula de múon com:

- **Posição e dimensões**: `x`, `y`, `z`, `dx`, `dy`, `dz`
- **Sinal**: `amplitude`, `time`
- **Identificação de múon**: `isMuon`, `theta`, `phi`
- **Variáveis de qualidade**: `eta`, `deltaEta`, `deltaPhi`, `energy`,
  `trackLength`, `recoTime`, `signal`, `efficiency`, `isTrigger`, `layer`,
  `samplingName`
- **Referência ao descriptor**: `descriptor`

Métodos principais:

- **`reset()`**: reinicia todas as variáveis dinâmicas (mantém posição e
  dimensões)
- **`merge(other)`**: combina informações de outra célula (soma de
  amplitudes/energias/sinais; médias ponderadas para tempo/theta/phi/eta;
  OR lógico para `isMuon` e `isTrigger`)

### `MuonChamberDescriptor` (`MuonChamberDetDescriptor.cxx`)

Representa o descriptor geométrico/físico de uma célula. Armazena:

- **Posição/dimensões**: `x`, `y`, `z`, `dx`, `dy`, `dz`
- **Identificação**: `hash`, `chamberType`, `sampling`, `layer`, `region`
- **Operação**: `efficiency`, `noiseLevel`, `timeResolution`,
  `spatialResolution`, `driftVelocity`, `operatingVoltage`, `gasMixture`
- **Energia/tempo**: `e` (estimada), `tau`
- **BCID**: `bc_duration`, `bcid_start`, `bcid_end`
- **Vetores por BCID**: `hit_count`, `drift_time`, `cluster_size`, `time`,
  `edep`, `tof`, e `pulse` (forma de onda)

Fornece construtores (default, completo e cópia), `copy()` para clonagem, e
getters/setters por BCID com validação de índice (`validateIndex`).

### `MuonChamberConverter` (`MuonChamberConverter.cxx`)

Conversor entre `MuonChamberCell*` e a struct POD `MuonChamber_t`:

- **`convert(cell*, cell_t&)`**: serializa célula para struct (incluindo
  campos do descriptor)
- **`convert(cell_t&, cell*&)`**: desserializa struct para célula
- **`convert(desc_t&, cell*&)`**: reconstrói célula a partir de um
  descriptor
- **`calcEta(x, y, z)`** e **`calcPhi(x, y)`**: utilitários para cálculo
  de η e φ a partir das coordenadas

### `MuonChamberDescriptorConverter` (`MuonChamberDescriptorConverter.cxx`)

Conversor entre `MuonChamberDescriptor*` e a struct POD
`MuonChamberDescriptor_t`:

- **`convert(desc*, desc_t&)`**: serializa descriptor, acumulando
  `truthEnergy`, `hitCount`, `clusterSize` e `driftTime` médios a partir
  dos BCIDs, e copiando o `pulse`
- **`convert(desc_t&, desc*&)`**: desserializa struct, criando um novo
  descriptor e preenchendo operação, energia estimada, verdade e hits no
  BCID central

### `LinkDef.h`

Registra no ROOT/CINT as structs POD (`MuonChamber_t`,
`MuonChamberDescriptor_t`), seus vetores e as classes principais
(`MuonChamberCell`, `MuonChamberDescriptor`), permitindo persistência em
arquivos ROOT e uso em PyROOT.

## Dependências

- **GaugiKernel**: Núcleo do framework (EDM, macros, DataVector)
- **MuonCell**: Definições das células de múons (este módulo)
- **MuonCluster**: Definições dos clusters de múons (referência)
- **ROOT** (`TTree`, `std::vector<...>`, `#pragma link`): Persistência e
  dicionário


O `MuonCell` fornece as **classes de dados** usadas em todos os estágios
seguintes: o `MuonCellBuilder` produz instâncias de `MuonChamberCell` +
`MuonChamberDescriptor`; o `MuonClusterBuilder` as agrupa; o `MuonBuilder` as
converte em `xAOD::Muon`.


