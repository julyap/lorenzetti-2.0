# 🧪 How to Add New Particles in Lorenzetti

This guide is intended for developers who want to add new particles to the **Lorenzetti** project. It details which files must be **created**, **modified**, and how to properly organize and link them.

---

## 📁 Files to Be Created

### 1. C++ Source File `.cxx` (particle generator)
Create at:  
```bash
generator/filters/src/MyParticle.cxx
```

### 2. Header `.h`
Create at:  
```bash
generator/filters/src/MyParticle.h
```

### 3. `.cmnd` File (event generator configuration)
Create at:  
```bash
generator/evtgen/myParticle.cmnd
```

### 4. Production Card `.json`
Create at:  
```bash
EVT/production cards/production_card_myParticle.json
```

Then, generate the jobs using the executable:

```bash
./create_jobs <production_card_path> <output_dir> <chunk_size> <MAX_EVENT_NUM> <particle_name>
```

### 5. Python Interface `particle.py`
Create at:  
```bash
generator/filters/python/particle.py
```
This file provides the Python interface for the new particle, importing the C++ bindings.

### 6. Python Generation Script
Create at:  
```bash
generator/scripts/gen_particle.py
```

### 7. Example Shell Script
Create at:  
```bash
generator/examples/gen_particle.sh
```
This script serves as an example of how to generate the particle within the project environment.

---

## 🛠 Files to Be Modified

### 1. `generator/filters/CMakeLists.txt`
Include the new `.h` and `.cxx` files in the build system.

---

## 🔗 C++ and Python Binding

Edit `LinkDef.h` and add the link between your header and the new particle class to ensure ROOT generates the required dictionaries.

---

## 🔃 Finally, recompile the project

Return to the root folder of Lorenzetti and run:
```bash
make
```

## 🗂 Expected Project Structure

```
lorenzetti/
├── generator/
│   ├── filters/
│   │   ├── src/
│   │   │   ├── MyParticle.cxx
│   │   │   └── MyParticle.h
│   │   └── python/
│   │       └── particle.py
│   ├── scripts/
│   │   └── gen_particle.py
│   └── examples/
│       └── gen_particle.sh
├── generator/
│   └── evtgen/
│       └── myParticle.cmnd
├── EVT/
│   └── production cards/
│       └── production_card_myParticle.json
```

>Made by Julya Pacheco
