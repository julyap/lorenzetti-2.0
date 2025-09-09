# 🚀 Lorenzetti – Tutorial de Instalação

📖 Read this in [English](README.md)

Este repositório contém o código do **Lorenzetti**, um projeto que roda dentro de um container Apptainer (Singularity). Este guia mostra como instalar as dependências, preparar o ambiente e compilar o projeto.

---
## 📦 Pré-requisitos

Instale os seguintes pacotes no seu sistema (recomendado para Ubuntu 22.04 ou superior):

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

---

## 🧊 Baixando a imagem do container

Você pode baixar a imagem `.sif` dividida em partes na página de releases do GitHub.

Após baixar todas as partes, reconstrua o container com:

```bash
cat lorenzetti.sif.part_* > lorenzetti.sif
```

---

## ▶️ Rodando o container

Abra o shell do container com:

```bash
apptainer run lorenzetti.sif
```

---

## 📥 Clonando o repositório

```bash
git clone https://github.com/julyap/lorenzetti-2.0/
cd lorenzetti
```

---

## 🛠️ Compilando o projeto

Dentro do container:

1. Vá para a pasta do projeto:
   ```bash
   cd /caminho/para/lorenzetti
   ```

2. Compile com:
   ```bash
   make
   ```

3. Configure o ambiente:
   ```bash
   source setup.sh
   ```

---

## ✅ Pronto!

Agora o Lorenzetti está instalado e pronto para uso dentro do container. Para dúvidas ou sugestões, abra uma issue neste repositório.

---


## 🎯 Como Rodar um Exemplo Rápido

Para testar o Lorenzetti, você pode rodar um evento de exemplo com a geração de uma partícula.

1. Dentro do container, navegue até a pasta de exemplos:
   ```bash
   cd generator/examples
   ```

2. Escolha o evento desejado 

3. Substitua o caminho para o seu diretorio nos locais apropriados

4. Execute o script correspondente:
   ```bash
   ./gen_(particula).sh
   ```

O script irá configurar e rodar a simulação daquela partícula.



## 🛠 Mais Ajuda

- 🧯 [Erros Comuns e Como Corrigir](como_resolver.md):  
  Saiba como lidar com erros de importação de módulos como `GaugiKernel` ou `reco`.

- 🧪 [Como Adicionar Novas Partículas](novas_particulas.md):  
  Tutorial passo a passo para desenvolvedores que desejam incluir novos geradores de partículas no framework.

- [Guia de Simulação e Pileup](simulação_pileup.md)  
  Guia passo a passo para desenvolvedores e usuários que desejam entender como o pipeline de simulação funciona no Lorenzetti e como incluir corretamente eventos de pileup.


> Feito por Julya Pacheco
