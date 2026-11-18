# Câmara de Múons — Guia de Uso

## O que é

O Lorenzetti foi estendido para suportar uma câmara de múons em paralelo ao
calorímetro. A escolha de quais detectores processar é controlada por um único
argumento em todos os scripts de transformação:

```bash
--systems {calorimeter,muon,atlas}    # default: calorimeter
```

| `--systems` | Calorímetro | Câmara de múons |
|---|---|---|
| `calorimeter` (default) | ✅ | ❌ |
| `muon` | ❌ | ✅ |
| `atlas` | ✅ | ✅ |

Sem a flag, o comportamento é **idêntico ao oficial** (só calorímetro). A flag
**não filtra partículas** — elétrons, múons, etc. continuam sendo simulados
conforme o arquivo de entrada. O que muda é **quais detectores produzem
hits/células/clusters/múons**.



## Como usar

### 1. Cadeia completa com múons

```bash
# Simulação
simu_trf.py  -i input.EVT.root  -o output.HIT.root  -nt 8 \
             --detector v2  --systems atlas

# (opcional) Overlay de pileup
merge_trf.py --low-pileup-files  low/  \
             --high-pileup-files high/ \
             --pileup-avg 40 --pileup-sigma 10 \
             --systems atlas

# Digitalização
digit_trf.py -i output.HIT.root  -o output.ESD.root  \
             --detector v2  --systems atlas

# Reconstrução
reco_trf.py  -i output.ESD.root  -o output.AOD.root  \
             --systems atlas

# Ntuple plano
ntuple_trf.py -i output.AOD.root  -o output.NTUPLE.root \
              --systems atlas
```

### 2. Apenas calorímetro (comportamento oficial)

```bash
simu_trf.py  -i input.EVT.root  -o output.HIT.root  -nt 8 --detector v2
digit_trf.py -i output.HIT.root  -o output.ESD.root  --detector v2
reco_trf.py  -i output.ESD.root  -o output.AOD.root
ntuple_trf.py -i output.AOD.root  -o output.NTUPLE.root
```

### 3. Apenas câmara de múons

```bash
simu_trf.py  -i input.EVT.root  -o output.HIT.root  -nt 8 \
             --detector v1  --systems muon

digit_trf.py -i output.HIT.root  -o output.ESD.root  \
             --detector v1  --systems muon

reco_trf.py  -i output.ESD.root  -o output.AOD.root  \
             --systems muon
```


## Regras importantes

1. **Use o mesmo `--systems` em todas as etapas da cadeia.** Se simular com
   `atlas` e digitalizar com `calorimeter`, os hits de múons serão ignorados.

2. **`merge_trf.py` não aplica pileup em múons.** No modo `atlas`, `Hits` é
   mesclado pelo `PileupMerge`, mas `MuonHits` passa direto do evento
   principal (pileup no espectrômetro de múons é desprezível).



4. **`--systems` é ortogonal a `--detector v1/v2`.** A geometria é escolhida
   por `--detector`; os detectores que produzem hits, por `--systems`.

---

## Chaves de recordables por etapa

| Etapa | Entrada | Saída |
|---|---|---|
| `simu_trf.py` | — | `Events`, `Particles`, `Seeds`, `Hits`, `MuonHits` |
| `merge_trf.py` | `Hits`, `MuonHits`, `Events`, `Particles`, `Seeds` + pileup | `Hits` (mesclado), `MuonHits` (passthrough), `Events`, `Particles`, `Seeds` |
| `digit_trf.py` | `Hits`, `MuonHits`, `Events`, `Particles`, `Seeds` | `Cells`, `TruthCells`, `MuonCells`, `TruthMuonCells`, `Events`, `Particles`, `Seeds` |
| `reco_trf.py` | `Cells`, `TruthCells`, `MuonCells`, `Events`, `Particles`, `Seeds` | `Clusters`, `TruthClusters`, `Rings*`, `TruthRings*`, `Electrons`, `TruthElectrons`, `MuonClusters`, `Muons` |
| `ntuple_trf.py` | AOD completo | `physics` (TTree plano, com branches `mu_*` no modo `atlas`) |

---

## Reconstrução de múons (detalhes)

O `MuonMaker` reconstrói múons MS-only em quatro categorias, controladas por
três variáveis: `nCells` (número de células no cluster), `chi2` (qualidade do
fit) e `fracMax` (concentração de energia, `emaxs2/totalEnergy`).

| Categoria | `min_cells` | `max_chi2` | `min_fracMax` |
|---|---|---|---|
| Very Loose | 1 | 999 | 0.5 |
| Loose | 2 | 30 | 0.6 |
| Medium | 2 | 15 | 0.8 |
| Tight-like | 2 | 6 | 0.9 |

Os cortes podem ser sobrescritos via `TightCuts`/`MediumCuts`/`LooseCuts`/
`VLooseCuts` no formato `[chi2_cut, min_cells, min_fracMax]`.

> ⚠️ O "Tight-like" é uma **aproximação MS-only** — o Tight oficial do ATLAS
> exige combinação ID+MS, que não é reproduzida apenas com cortes do
> espectrômetro de múons.

---

## Ntuple: branches de múons (modo `atlas`)

Quando `--systems atlas`, o `ntuple_trf.py` adiciona ao TTree `physics`:

- `mu_n` — número de múons no evento
- `mu_pt` — lista de pT (GeV)
- `mu_eta` — lista de η
- `mu_phi` — lista de φ
- `mu_e` — lista de energia (GeV)
- `mu_charge` — lista de carga
- `mu_author` — lista de `author` (tipo de múon)

As branches de calorímetro/elétron permanecem inalteradas.
