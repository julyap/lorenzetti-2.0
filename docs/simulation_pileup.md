# 🔧 Lorenzetti – Simulation and Pileup Guide

This document explains the five fundamental steps of the simulation pipeline in **Lorenzetti**, as well as how to include pileup events.

---

## 🧪 Pipeline: Five Main Steps

The Lorenzetti pipeline follows **five main stages**:

1. **Event Generation**
2. **Detector Propagation (Simulation)**
3. **Digitalization**
4. **Event Reconstruction**

> 🔹 These steps are automatically performed by the job system.
> You can find the generated jobs in the directories:
>
> * Without pileup:
>   `/home/administrador/apptainer-1.3.0/lorenzetti/EVT/jobs/jobs_zee`
>
> * With pileup:
>   `/home/administrador/apptainer-1.3.0/lorenzetti/EVT/jobs/jobs_jpsi`

---

### 1. Event Generation

Particles are generated using event filters located in `generator/filters/scripts`. You can choose the physical process or decay mode and include pileup conditions (`--pileupAvg`).

**Example:**

```bash
prun_jobs.py -c "gen_zee.py --pileupAvg 0 --nov %NOV --eventNumber %OFFSET -o %OUT -s %SEED" -nt 40 --nov $NOV --seed $SEED --novPerJob 200 -o EVT.root
```

You can also use external generators (e.g., Pythia8) and convert the output to HepMC format to be used in the Lorenzetti framework.

---

### 2. Detector Propagation (Simulation)

The generated particles are propagated through the detector using the **Geant4** module, which simulates their interaction with the geometry.

**Example:**

```bash
simu_trf.py -i EVT.root -o HIT.root --enableMagneticField
```

---

### 3. Digitalization

This step converts energy deposits into electronic signals, with optional simulation of effects like crosstalk.

**Example:**

```bash
digit_trf.py -i HIT.root -o ESD.root
```

---

### 4. Event Reconstruction

In this stage, physical objects such as clusters and particles are reconstructed from the digitized data.

**Example:**

```bash
reco_trf.py -i ESD.root -o AOD.root
```

---

## ➕ How to Simulate Pileup Events

To properly simulate pileup:

---

### 1. Generate Two Sets of Events:

* **Main**: the signal of interest (e.g., J/ψ → μ⁺μ⁻)
* **Pileup**: minimum bias events

Use the script:

```bash
python gen_minbias.py -o pileup.root --nov 100 --run-number 12345 -s 42 --pileup-avg 40 --pileup-sigma 5
```

---

### 2. Simulate Both Sets with `simu_trf.py`:

```bash
simu_trf.py -i principal.root -o principal.HIT.root
simu_trf.py -i pileup.root -o pileup.HIT.root
```

---

### 3. Merge the Hits:

```bash
merge_trf.py --input-file principal.HIT.root --pileup-file pileup.HIT.root -o merged.HIT.root --nov 100
```

---

### 4. Proceed with Digitalization and Reconstruction:

```bash
digit_trf.py -i merged.HIT.root -o ESD.root
reco_trf.py -i ESD.root -o AOD.root
```

---

> Made by Julya Pacheco

