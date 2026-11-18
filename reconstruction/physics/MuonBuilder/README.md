# MuonBuilder

## Visão Geral

Este diretório contém o módulo **MuonBuilder**, responsável pela reconstrução
final de múons no sistema de detecção. Este componente é parte do pipeline de
reconstrução de eventos, processando os clusters de múons produzidos pelo
`MuonClusterBuilder` para gerar os múons reconstruídos que serão utilizados na
análise física.

A reconstrução aqui implementada é **MS-only** (apenas Muon Spectrometer, sem
informação do Inner Detector), com cortes baseados no ATLAS 2016
(arXiv:1603.05598). As categorias de qualidade são: Very Loose, Loose, Medium e
**Tight-like** (uma aproximação para MS-only, **não** o Tight oficial do ATLAS,
que exige combinação ID+MS).

## Estrutura do Diretório
MuonChamber/
├── MuonBuilder/ # Módulo atual - Builder de múons
│ ├── src/ # Código fonte
│ │ ├── *.cxx # Arquivos de implementação
│ │ ├── *.h # Arquivos de cabeçalho
│ │ └── LinkDef.h # Definições para geração de dicionário ROOT
│ ├── python/ # Módulos Python de configuração
│ │ ├── CutBasedMuonID.py # Definição dos cortes por working point
│ │ ├── MuonBuilder.py # Função de configuração do builder
│ │ └── MuonMaker.py # Wrapper Python do algoritmo C++
│ ├── CMakeLists.txt # Configuração de build
│ └── README.md # Este arquivo


## Funcionalidade

O `MuonBuilder` é responsável por:

1. **Reconstrução de múons MS-only**: Aplica cortes de qualidade aos clusters
   de múons para produzir a coleção final de múons (`Muons`)
2. **Classificação por qualidade**: Atribui a cada múon uma categoria
   (Very Loose, Loose, Medium ou Tight-like) com base em três variáveis:
   - `nCells`: número de células/camadas no cluster
   - `chi2`: qualidade do fit da track no MS
   - `fracMax`: concentração de energia (`emaxs2 / totalEnergy`), usada para
     discriminar assinaturas de MIP
3. **Cálculo de variáveis derivadas**: Estima η, φ, pT, E, e armazena fracMax
   e chi2 no objeto `xAOD::Muon`, junto com as flags de qualidade
4. **Preparação para análise**: Produz uma coleção estruturada de múons com
   todas as variáveis de qualidade preenchidas

## Componentes

### `CutBasedMuonID`

Define os cortes de identificação de múons por working point, inspirados na
performance do ATLAS 2016. Cada working point (`vloose`, `loose`, `medium`,
`tight`, `highpt`, `lowpt`) especifica valores para:

- Compatibilidade q/p (`qpCompatibilityCut`)
- ρ' (`rhoCut`)
- χ²/ndof (`chi2Cut`)
- Número mínimo de precision stations (`precisionStationsCut`)
- MBS, SNS, SCS (`mbsCut`, `snsCut`, `scsCut`)
- fracMax (`fracMaxCut`)
- Hits no pixel/SCT e hits perdidos
- Significâncias de d0 e z0·sinθ
- Tipos de múon aceitos (CB, IO, ME, SiF, ST, CT)
- Thresholds em pT

> **Nota:** o working point `tight` definido aqui é uma aproximação
> **Tight-like** para MS-only. O Tight oficial do ATLAS exige combinação ID+MS.

### `MuonBuilder`

Função de configuração `MuonBuilderCfg` que instancia e configura o `MuonMaker`
com os cortes das quatro categorias MS-only. Os cortes são passados no formato
`[chi2_cut, min_cells, min_fracMax]`:

| Categoria   | Min Cells | Max Chi2 | Min FracMax | Base ATLAS 2016                    |
|-------------|-----------|----------|-------------|-------------------------------------|
| Very Loose  | ≥ 1       | < 999    | ≥ 0.5       | CT muons em \|η\|<0.1 (MIP)         |
| Loose       | ≥ 2       | < 30     | ≥ 0.6       | MS track padrão (≥2 segmentos)      |
| Medium      | ≥ 2       | < 15     | ≥ 0.7       | MS-only de boa qualidade            |
| Tight-like  | ≥ 3       | < 8      | ≥ 0.8       | Aprox. ME forward / Tight CB (χ²<8) |

### `MuonMaker`

Wrapper Python/C++ do algoritmo de reconstrução de múons. Recebe a coleção de
clusters de múons (`InputMuonClusterKey`) e produz a coleção de múons
reconstruídos (`OutputMuonKey`). Cada cluster que passa em pelo menos uma
categoria gera um `xAOD::Muon` com:

- `η`, `φ`, `pT`, `E`
- `quality` (0 = Very Loose, 1 = Loose, 2 = Medium, 3 = Tight-like)
- flags `isVeryLoose`, `isLoose`, `isMedium`, `isTight`
- `fracMax`, `chi2`
- referência ao cluster original (`muonCluster()`)

## Cortes por Categoria (MS-only)

A tabela abaixo resume os cortes usados por cada categoria, conforme
implementado em `MuonMaker.cxx` (defaults) e configurável via
`MuonBuilderCfg`:

| Categoria   | nCells mínimo | χ² máximo | fracMax mínimo |
|-------------|---------------|-----------|----------------|
| Very Loose  | 1             | 999.0     | 0.5            |
| Loose       | 2             | 30.0      | 0.6            |
| Medium      | 2             | 15.0      | 0.7            |
| Tight-like  | 3             | 8.0       | 0.8            |

> ⚠️ **Tight-like é uma aproximação para MS-only.** O Tight oficial do ATLAS
> requer combinação de informação do Inner Detector (ID) e do Muon
> Spectrometer (MS).

## Dependências

Este módulo depende de outros componentes do sistema de reconstrução:

- **GaugiKernel**: Núcleo do framework de processamento
- **MuonCluster**: Definições dos clusters de múons (entrada do builder)
- **Muon**: Definições dos múons reconstruídos (saída do builder)
- **ROOT** (`TH1F`, `TH2F`): Histogramas de controle

## Fluxo no Pipeline

O `MuonBuilder` consome os clusters produzidos pelo `MuonClusterBuilder` e
entrega múons reconstruídos prontos para os estágios seguintes de análise.


  `muon_chi2_vs_fracmax`

## Referências

- ATLAS Collaboration, "Muon reconstruction performance of the ATLAS detector
  in proton-proton collision data at √s = 13 TeV",
  Eur. Phys. J. C 76 (2016) 292, arXiv:1603.05598
  - Seção 3.1: MS track reconstruction (≥2 segmentos)
  - Seção 5.1: ME muons para |η|>2.5 (≥3 camadas)
  - Seção 5.1.2: Tight CB muons com χ² < 8
  - Tabela 1: eficiências Loose 96.7–98.1%, Tight 89.9–91.8%




