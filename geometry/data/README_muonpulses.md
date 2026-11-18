# Geração de Pulse Shapes das Câmaras de Múon do ATLAS

## Visão Geral

Este diretório contém o script `generate_pulse.py`, responsável por gerar as
**formas de onda (pulse shapes)** das quatro tecnologias de câmaras de múon
do ATLAS:

- **MDT** (Monitored Drift Tube)
- **CSC** (Cathode Strip Chamber)
- **TGC** (Thin Gap Chamber)
- **RPC** (Resistive Plate Chamber)

Cada pulso é parametrizado com base nos dados dos respectivos **TDRs**
(Technical Design Reports) e salvo em um arquivo `.dat` no formato de duas
colunas (`time amplitude`), que é lido diretamente pelo
`MuonPulseGenerator` durante a digitalização.

## Arquivos Gerados

| Arquivo | Câmara | Range (ns) | Passo (ns) | Rise | Decay | Pico |
|---------|--------|------------|------------|------|-------|------|
| `pulseMDT.dat` | MDT | -100 a 2000 | 10 | até 300 ns | a partir de 300 ns | 300 ns |
| `pulseCSC.dat` | CSC | -50 a 200 | 2 | até 16 ns | a partir de 16 ns | 16 ns |
| `pulseTGC.dat` | TGC | -10 a 20 | 0.2 | até 3.5 ns | a partir de 3.5 ns | 3.5 ns |
| `pulseRPC.dat` | RPC | -10 a 21 | 0.2 | até 1.5 ns | a partir de 1.5 ns | 1.5 ns |

### MDT — `pulseMDT.dat`

- **Gás**: Ar-N₂-CH₄ (91/4/5%), 3 bar, 3270 V
- **Drift máximo**: 480 ns
- **Resolução espacial**: 80 µm
- **Modelo**: rise exponencial (τ = 80 ns) até 300 ns, decay exponencial
  (τ = 350 ns) a partir daí.
- **Ruído**: gaussiano de 2% quando a amplitude > 0.01.

### CSC — `pulseCSC.dat`

- **Gás**: Ar-CO₂-CF₄ (30/50/20%), 2600 V
- **Espaçamento do cátodo**: 5.08 mm
- **Pitch do fio**: 2.54 mm
- **Modelo**: rise exponencial (τ = 4 ns) até 16 ns, decay exponencial
  (τ = 15 ns) a partir daí.

### TGC — `pulseTGC.dat`

- **Gás**: CO₂/n-C₅H₁₂ (55/45%), 3100 V
- **Gap**: 2.8 mm
- **Ganho**: 10⁶
- **Eficiência**: > 99% em 25 ns
- **Modelo**: rise exponencial (τ = 1 ns) até 3.5 ns, decay exponencial
  (τ = 5 ns) a partir daí.
- **Ruído**: gaussiano de 2% quando a amplitude > 0.01.
- **Observação**: o formato foi **modificado para ser compatível com o RPC**
  (mesmo passo de 0.2 ns e mesmo range), para que o `MuonPulseGenerator`
  possa lê-lo sem ajustes.

### RPC — `pulseRPC.dat`

- **Gás**: C₂H₂F₄-C₄H₁₀ (97/3%), 8900 V
- **Gap**: 2.0 mm
- **Resolução temporal**: 1.5 ns RMS
- **Modelo**: rise exponencial (τ = 0.5 ns) até 1.5 ns, decay exponencial
  (τ = 1.2 ns) a partir daí.


