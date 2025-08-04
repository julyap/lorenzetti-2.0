# 🚀 Lorenzetti – Installation Tutorial

📖 Leia isso em [Português](README.pt-BR.md)

This repository contains the code for **Lorenzetti**, a project that runs inside an Apptainer (Singularity) container. This guide explains how to install dependencies, prepare the environment, and build the project.

---

## 📦 Requirements

- [Go](https://golang.org/)
- [Apptainer](https://apptainer.org/) (or Singularity)

On Ubuntu:

```bash
sudo apt update
sudo apt install golang-go apptainer -y
```

---

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
apptainer shell --bind $(pwd) lorenzetti.sif
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

To quickly test Lorenzetti, you can run an example event that generates a particle.

1. Inside the container, go to the examples directory:
   ```bash
   cd generator/examples
   ```

2. Choose the particle/event you want to run (e.g., `jpsi`, `Zee`, `muon`, etc).

3. Execute the corresponding script:
   ```bash
   ./gen_particula.sh
   ```

The script will set up and run the simulation for that particle.

## 🛠 More Help

- 🧯 [Common Errors and How to Fix Them](TROUBLESHOOTING.md):  
  Learn how to deal with missing modules like `GaugiKernel` or `reco`, using `sys.path`, or installing with `setup.py`.

- 🧪 [How to Add New Particles](new_particles.md):  
  Step-by-step guide for developers who want to introduce new particle generators in the framework.

- [Simulation and Pileup Guide](simulation_pileup.md)
   Step-by-step guide for developers and users who want to understand how the simulation pipeline works in Lorenzetti and how to correctly include pileup events.

> Made by Julya Pacheco
