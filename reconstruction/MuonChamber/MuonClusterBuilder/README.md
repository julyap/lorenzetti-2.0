# MuonClusterBuilder

## Visão Geral

Este diretório contém o módulo **MuonClusterBuilder**, responsável pelo agrupamento
(clustering) de células de muons e pelo cálculo das variáveis de qualidade e de
tracking dos clusters reconstruídos. Este componente é parte do pipeline de
reconstrução de eventos, processando as células de muons produzidas pelo
`MuonCellBuilder` para gerar clusters de muons que serão posteriormente utilizados
na identificação e reconstrução de trajetórias.

## Estrutura do Diretório
MuonChamber/
├── MuonClusterBuilder/ # Módulo atual - Builder de clusters de muons
│ ├── src/ # Código fonte
│ │ ├── *.cxx # Arquivos de implementação
│ │ ├── *.h # Arquivos de cabeçalho
│ │ └── LinkDef.h # Definições para geração de dicionário ROOT
│ ├── CMakeLists.txt # Configuração de build
│ └── README.md # Este arquivo

text

## Funcionalidade

O `MuonClusterBuilder` é responsável por:

1. **Agrupamento de células**: Agrupa células de muons em clusters com base em
   janelas de proximidade em η/φ e em X/Y/Z, usando como sementes as células
   com energia acima de um limiar configurável
2. **Cálculo de variáveis de forma e qualidade**: Calcula `fracMax`, χ²,
   q/p significance e ρ' para cada cluster, além de aplicar cortes de qualidade
   (VeryLoose, Loose, Medium e Tight) inspirados na seleção de múons do ATLAS 2016
3. **Estimativa de parâmetros de tracking**: Calcula carga, pT, p, d0 e z0 a
   partir das células do cluster, incluindo um fit de hélice quando há pontos
   suficientes
4. **Preparação para análise**: Produz uma coleção estruturada de clusters
   (`MuonClusters`) com todas as variáveis de qualidade e tracking preenchidas,
   pronta para os próximos estágios da reconstrução

## Componentes

### `MuonClusterFlags`

Define as flags globais de configuração do clustering, com valores default
otimizados para múons (ATLAS 2016):

- `MinCenterEnergy`: energia mínima para considerar uma célula como semente
- `EtaWindow` / `PhiWindow`: janelas de agrupamento em η e φ
- `XWindow` / `YWindow` / `ZWindow`: janelas espaciais em mm
- `TimeWindow`: janela temporal para agrupamento
- `DoForwardMoments`, `DoCurvatureCalc`, `DoTimeAnalysis`, `DoLayerEfficiency`:
  flags de processamento

### `MuonClusterMaker`

Wrapper Python/C++ do algoritmo de clustering. Recebe a coleção de células
(`InputCellsKey`) e a coleção de sementes (`InputSeedsKey`), e produz a coleção
de clusters (`OutputClusterKey`). Permite configurar as janelas de agrupamento,
a energia mínima da semente, o uso de centroide ponderado por energia e o
cálculo de momentos direcionais. Opcionalmente, pode acoplar uma `ShapesTool`
para caracterização de forma.

### `MuonShapes`

`AlgTool` responsável por calcular as variáveis de qualidade e de tracking de
cada cluster:

- **Tracking**: `charge`, `pT`, `p`, `d0`, `z0`, `trackChi2` (via
  `performTracking`, com estimativas auxiliares e fit de hélice)
- **Qualidade**: `fracMax`, χ², q/p significance, ρ'
- **Cortes**: `isVeryLoose`, `isLoose`, `isMedium`, `isTight` (e um código
  inteiro `quality` de 0 a 3)

## Dependências

Este módulo depende de outros componentes do sistema de reconstrução:

- **GaugiKernel**: Núcleo do framework de processamento
- **MuonCell**: Definições das células de muons (entrada do clustering)
- **MuonCluster**: Definições dos clusters de muons (saída do clustering)
- **MuonChamber**: Definições das câmaras de muons
- **ROOT** (`TVector3`, `TMatrixD`, `TMatrixDEigen`): Utilidades matemáticas
  usadas no fit de hélice e nos cálculos de forma

O `MuonClusterBuilder` consome as células produzidas pelo `MuonCellBuilder` e
entrega clusters prontos para os estágios seguintes de reconstrução e análise.
