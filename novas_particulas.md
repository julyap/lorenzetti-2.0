# 🧪 Como Adicionar Novas Partículas no Lorenzetti

Este guia é voltado para desenvolvedores que desejam adicionar novas partículas ao projeto **Lorenzetti**. Ele descreve detalhadamente quais arquivos devem ser **criadas**, **modificados** e como organizá-los corretamente.

---

## 📁 Arquivos que Devem Ser Criados

### 1. Arquivo C++ `.cxx` (gerador da partícula)
Crie em:  
```bash
generator/filters/src/MinhaParticula.cxx
```
Este arquivo deve conter a implementação da lógica de geração da partícula, herdando de `GeneratorBase` (ou estrutura equivalente), com definição de massa, carga, spin etc.

### 2. Header `.h`
Crie em:  
```bash
generator/filters/src/MinhaParticula.h
```
Este cabeçalho define a interface da nova classe geradora da partícula.

### 3. Arquivo `.cmnd` (configuração do gerador de eventos)
Crie em:  
```bash
generator/evtgen/minhaParticula.cmnd
```

### 4. Cartão de Produção (`.json`)
Crie em:  
```bash
EVT/production cards/production_card_minhaParticula.json
```

Depois, crie os jobs usando o executável:

```bash
./create_jobs <production_card_path> <output_dir> <chunk_size> <MAX_EVENT_NUM> <nome_da_particula>
```

### 5. Módulo Python `particula.py`
Crie em:  
```bash
generator/filters/python/particula.py
```
Este arquivo contém a interface Python da nova partícula, conectada aos bindings em C++.

### 6. Script de Geração em Python
Crie em:  
```bash
generator/scripts/gen_particula.py
```

### 7. Script Shell de Exemplo
Crie em:  
```bash
generator/examples/gen_particula.sh
```
Este script serve como exemplo para rodar a geração da partícula no ambiente do projeto.

---

## 🛠 Arquivos que Devem Ser Modificados

### 1. `generator/filters/CMakeLists.txt`
Inclua o novo header `.h` e o `.cxx` no sistema de build.

---

## 🔗 Ligação entre C++ e Python (Bindings)

Entre em `LinkDef.h` e adicione o link entre o seu header e a classe da nova partícula para garantir que o ROOT gere corretamente os dicionários.

---

## 🔃 Por fim, recompile o projeto

Volte à pasta principal do Lorenzetti e execute:

```bash
make
```

## 🗂 Estrutura Esperada do Projeto

```
lorenzetti/
├── generator/
│   ├── filters/
│   │   ├── src/
│   │   │   ├── MinhaParticula.cxx
│   │   │   └── MinhaParticula.h
│   │   └── python/
│   │       └── particula.py
│   ├── scripts/
│   │   └── gen_particula.py
│   └── examples/
│       └── gen_particula.sh
├── generator/
│   └── evtgen/
│       └── minhaParticula.cmnd
├── EVT/
│   └── production cards/
│       └── production_card_minhaParticula.json

```

> Feito por Julya Pacheco
