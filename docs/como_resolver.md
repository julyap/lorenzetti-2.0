# 🧯 Erros Comuns e Como Corrigir

Este documento lista problemas frequentes que podem ocorrer ao usar ou desenvolver com o **Lorenzetti**, e como resolvê-los.

---

## ❌ Erro: Módulo Python não encontrado

**Exemplos:**

```
ModuleNotFoundError: No module named 'GaugiKernel'
```

ou

```
ModuleNotFoundError: No module named 'reco'
```

### 🔍 Por que isso acontece?

Esse erro geralmente ocorre quando:
- O módulo Python não foi instalado
- O caminho não está no `PYTHONPATH`
- O módulo não está vinculado corretamente via `setup.py`

---

### ✅ Solução 1: Adicionar o módulo manualmente com `sys.path` (funciona para todos os casos)

No seu script ou terminal Python, faça:

```python
import sys
sys.path.append('/caminho/absoluto/para/o/modulo')  # Altere com o caminho correto
```

Isso serve para **qualquer módulo**, incluindo `GaugiKernel`, `reco` ou seus próprios scripts.

---

### ✅ Solução 2: Instalar com `setup.py` (mais robusta)

Se a solução acima não resolver, ou se você quiser algo permanente, crie um arquivo `setup.py` no diretório do módulo (por exemplo, `reco/`), parecido com o que já existe em `GaugiKernel`:

```python
from setuptools import setup

setup(
    name='reco',
    version='0.1',
    packages=['reco'],
)
```

Depois, dentro do container Apptainer:

```bash
# Crie um diretório temporário gravável
export PYTHONUSERBASE=/tmp/pip_install
mkdir -p $PYTHONUSERBASE

# Instale o módulo
pip install --prefix=$PYTHONUSERBASE --no-deps -e .

# Adicione ao PYTHONPATH
export PYTHONPATH="$PYTHONUSERBASE/lib/python3.10/site-packages:$PYTHONPATH"
```

Por fim, teste o módulo:

```python
import reco
```

---

Outros erros e soluções serão adicionados conforme necessário.

