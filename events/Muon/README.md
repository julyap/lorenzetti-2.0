# Muon

## Visão Geral

Este diretório contém a implementação da classe **`xAOD::Muon`**, que
representa um múon reconstruído no Event Data Model (EDM) do ATLAS. A classe
herda de `Gaugi::EDM` e armazena todas as variáveis relevantes para análise
e seleção de múons, incluindo:

- variáveis cinemáticas (η, φ, E, ET, pT, carga);
- variáveis de qualidade e isolamento;
- parâmetros de impacto (d0, z0, σd0, σz0);
- variáveis de qualidade herdadas do cluster de origem (fracMax, χ²,
  q/p significance, ρ');
- flags dos working points (VeryLoose, Loose, Medium, Tight-like);
- referência ao cluster de origem (`MuonCluster`).

O módulo também fornece o container padrão (`MuonContainer`) e a infraestrutura
de persistência (`Muon_t` + `MuonConverter`) para escrita/leitura em arquivos
ROOT.

## Arquivos

| Arquivo | Descrição |
|---------|-----------|
| `Muon.h` | Declaração da classe `xAOD::Muon` (getters/setters, flags, cluster) |
| `Muon.cxx` | Implementação dos construtores da classe `xAOD::Muon` |
| `MuonContainer.h` | Typedef `xAOD::MuonContainer` = `SG::DataVector<xAOD::Muon>` |
| `MuonConverter.h` | Declaração da struct POD `Muon_t` e da classe `MuonConverter` |
| `MuonConverter.cxx` | Implementação das conversões `Muon ↔ Muon_t` |

