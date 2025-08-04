# 🔧 Lorenzetti – Guia de Simulação e Pileup

Este documento explica os cinco passos fundamentais do pipeline de simulação no Lorenzetti, além de como incluir eventos de pileup.

---

## 🧪 Pipeline: Cinco Etapas Principais

O pipeline do Lorenzetti segue **cinco estágios principais**:

1. **Geração de Eventos**  
2. **Propagação no Detector (Simulação)**   
3. **Digitalização**  
4. **Reconstrução**


> 🔹 Os **passos** são realizados automaticamente pelos jobs gerados no sistema.  
> Você pode encontrar os jobs prontos nos diretórios:
>
> - Sem pileup:  
>   `/home/administrador/apptainer-1.3.0/lorenzetti/EVT/jobs/jobs_zee`
>
> - Com pileup:  
>   `/home/administrador/apptainer-1.3.0/lorenzetti/EVT/jobs/jobs_jpsi`

---

---

### 1. Geração de Eventos

As partículas são geradas usando filtros de eventos localizados em `generator/filters/scripts`. Você pode escolher o processo físico ou modo de decaimento, e incluir condições de pileup (`--pileupAvg`).

**Exemplo:**
```bash
prun_jobs.py -c "gen_zee.py --pileupAvg 0 --nov %NOV --eventNumber %OFFSET -o %OUT -s %SEED" -nt 40 --nov $NOV --seed $SEED --novPerJob 200 -o EVT.root
```
Também é possível usar geradores externos (como Pythia8) e converter para o formato HepMC para uso no framework.

### 2. Propagação no Detector (Simulação)

As partículas geradas são propagadas no detector utilizando o módulo Geant4, que simula as interações com a geometria.

**Exemplo:**
```bash
simu_trf.py -i EVT.root -o HIT.root --enableMagneticField
```

### 3. Digitalização

Converte os depósitos de energia em sinais eletrônicos, com simulação opcional de efeitos como crosstalk.

**Exemplo:**
```bash
digit_trf.py -i HIT.root -o ESD.root
```

### 4. Reconstrução de Eventos

Reconstrói objetos físicos (clusters, partículas) a partir dos dados digitalizados.

**Exemplo:**
```bash
reco_trf.py -i ESD.root -o AOD.root
```
## ➕ Como Fazer Um Evento Com Pileup

Para simular pileup corretamente:

###    Gere dois conjuntos de eventos:

        Principal: sinal físico de interesse (ex: J/ψ → μ⁺μ⁻)

        Pileup: eventos de colisão mínima (min bias)  
            Use o script:
            ```bash
            gen_minbias.py e o comando python gen_minbias.py -o pileup.root --nov 100 --run-number 12345 -s 42 --pileup-avg 40 --pileup-sigma 5
            ```
###    Simule ambos com simu_trf.py:
```bash
simu_trf.py -i principal.root -o principal.HIT.root
simu_trf.py -i pileup.root -o pileup.HIT.root
```
###    Mescle os hits:
```bash
merge_trf.py --input-file principal.HIT.root --pileup-file pileup.HIT.root -o merged.HIT.root --nov 100
```

###    Prossiga com digitalização e reconstrução usando o arquivo mesclado.

> Feito por Julya Pacheco
