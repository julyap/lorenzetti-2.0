# 🚀 Lorenzetti – Installation Tutorial

📖 Leia isso em [Português](README.pt-BR.md)

This repository contains the code for **Lorenzetti**, a project that runs inside an Apptainer (Singularity) container. This guide explains how to install dependencies, prepare the environment, and build the project.

---

## 📦 Requirements

```bash
sudo apt update
sudo apt install -y \
  golang-go \
  apptainer \
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
  python3.10-dev \
  python3.10 \
  libpython3.10-dev
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
🎯 Running a Quick Example

To test Lorenzetti, you can run a sample particle generation event:

1. Inside the container, go to the production cards folder:
  
    ```bash
    cd EVT/production_cards
   ```
2. Create jobs using the executable:

   ```bash
   ./create_jobs <production_card_path> <output_dir> <chunk_size> <MAX_EVENT_NUM> <particle_name>
   ```
3. Then go to the examples folder:

   ```bash
   cd generator/examples
   ```
4. Choose the desired event.

5. Replace the paths in the script with your own directory structure where needed.

6. Run the corresponding script:
   ```bash
    ./gen_particula.sh
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
