# 🧯 Common Errors and How to Fix Them

This document lists common problems you may encounter when using or developing with **Lorenzetti**, and how to resolve them.

---

## ❌ Error: Python module not found

**Example:**

```
ModuleNotFoundError: No module named 'GaugiKernel'
```

or

```
ModuleNotFoundError: No module named 'reco'
```

### 🔍 Why it happens

This usually happens when a Python module is:
- Not installed
- Not in your `PYTHONPATH`
- Not linked correctly with `setup.py`

---

### ✅ Solution 1: Add the module to `sys.path` (quick fix for all cases)

In your script or Python terminal:

```python
import sys
sys.path.append('/absolute/path/to/module')  # Replace with the correct path
```

This works for **any module**, including `GaugiKernel`, `reco`, or custom generators.

---

### ✅ Solution 2: Full installation using `setup.py` (recommended if persistent)

If `sys.path` is not enough or if you want a more robust solution:

1. Create a `setup.py` file in the module directory, for example in `reco/`, similar to the one used in `GaugiKernel`.

```python
from setuptools import setup

setup(
    name='reco',
    version='0.1',
    packages=['reco'],
)
```

2. Then, inside the Apptainer container:

```bash
# Create a writable temp directory
export PYTHONUSERBASE=/tmp/pip_install
mkdir -p $PYTHONUSERBASE

# Install the module
pip install --prefix=$PYTHONUSERBASE --no-deps -e .

# Add it to the PYTHONPATH
export PYTHONPATH="$PYTHONUSERBASE/lib/python3.10/site-packages:$PYTHONPATH"
```

3. Test the module:

```python
import reco
```

---

More solutions and errors will be added as needed.
