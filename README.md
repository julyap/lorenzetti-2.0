
# Lorenzetti Simulator

Lorenzetti is a framework for the HEP community to freely exploit the full potential of calorimetry data. We expect to enable the community to mitigate bottlenecks for R&D in processing algorithms using calorimetry data by providing:

 - Unified low-level calorimetry and physics information based on full simulation (geant);
 - Free-to-use data. 
 
In other words, it provides a way for the HEP community to work on proof-of-concepts (POCs) using simulated data that is currently difficult to obtain on experiments and to publish them independently. We believe that this possibility, i.e. to publish POCs apart from the experiments, can be a powerful way to foster scientific exchange within the HEP community, but also to facilitate the exchange of processing algorithms with the broader scientific community.

We welcome everyone to contribute!


## Citations

Please cite [![DOI](https://zenodo.org/badge/370090835.svg)](https://zenodo.org/badge/latestdoi/370090835) if you use the software.

[//]: # (and/or the applicable papers.)


## Detector Construction:

The standard detector in the Lorenzetti framework consist in a eletromagnetic calorimeter and a hadronic calorimeter using a cylinder shape. Each one has 3 layers with different granularities to capture the shower develop by the particles. Also, between regions, there is a small slice of dead material.


![Screenshot](geometry/ATLAS/doc/cut_view.png)

It is possible, by using Geant4 modules to change the geometry, the layers and the cell granularity, allowing a high level of customization of the full detector.


## 📚 Documentation

- 📖 [README (English)](docs/README.md)
- 📘 [README (Português)](docs/README.pt-BR.md)
- 🧪 [How to Add New Particles](docs/new_particles.md)
- 🧪 [Como Adicionar Novas Partículas](docs/novas_particulas.md)
- 🌩️ [Troubleshooting (English)](docs/TroubleShooting.md)
- 🧯 [Como Resolver Problemas Comuns](docs/como_resolver.md)
- 🌀 [Simulation and Pileup (English)](docs/simulation_pileup.md)
- 🌀 [Simulação e Pileup (Português)](docs/simulação_pileup.md)

=======
# 🚀 Lorenzetti – Installation Tutorial

📖 Leia isso em [Português](README.pt-BR.md)

This repository contains the code for **Lorenzetti**, a project that runs inside an Apptainer (Singularity) container. This guide explains how to install dependencies, prepare the environment, and build the project.

---

## 📦 Requirements

```bash
sudo apt update
sudo apt install -y \
  golang-go \
  build-essential \
  cmake \
  libexpat1-dev \
  zlib1g-dev \
  libxerces-c-dev \
  libgl1-mesa-dev \
  libx11-dev \
  libxext-dev \
  libice-dev \
  libsm-dev \
  python3 \
  python3-dev \
  libpython3-dev
  libseccomp-dev \
  pkg-config \
  squashfs-tools \
  cryptsetup \
  runc \
  uidmap \
  git
```

## 📥 Clone the repository

```bash
git clone https://github.com/julyapacheco/lorenzetti.git
cd lorenzetti
```

---

## 🧊 Download the container image

You can download the `.sif` container in multiple parts from the GitHub release page.

After downloading all parts, reconstruct the full container using:

```bash
cat lorenzetti.sif.part_* > lorenzetti.sif
```

---

## ▶️ Running the container

Start the container shell with:

```bash
apptainer run lorenzetti.sif
```

---

## 🛠️ Building the project

Inside the container:

1. Go to the project folder:
   ```bash
   cd /path/to/lorenzetti
   ```

2. Compile:
   ```bash
   make
   ```

3. Set up the environment:
   ```bash
   source setup.sh
   ```

---

## ✅ Done!

Lorenzetti is now ready to use inside the container. If you have questions or suggestions, feel free to open an issue.

---

## 🎯 How to Run a Quick Example
🎯 Running a Quick Example

To test Lorenzetti, you can run a sample particle generation event:

1. Inside the container, go to the examples folder:

   ```bash
   cd generator/examples
   ```
2. Choose the desired event.

3. Replace the paths in the script with your own directory structure where needed.

4. Run the corresponding script:
   ```bash
    ./gen_(particle).sh
   ```
   
The script will configure and run the simulation for that specific particle.

## 🛠 More Help

- 🧯 [Common Errors and How to Fix Them](TROUBLESHOOTING.md):  
  Learn how to deal with missing modules like `GaugiKernel` or `reco`, using `sys.path`, or installing with `setup.py`.

- 🧪 [How to Add New Particles](new_particles.md):  
  Step-by-step guide for developers who want to introduce new particle generators in the framework.

- [Simulation and Pileup Guide](simulation_pileup.md)
   Step-by-step guide for developers and users who want to understand how the simulation pipeline works in Lorenzetti and how to correctly include pileup events.

> Made by Julya Pacheco
>>>>>>> atualizando docs
