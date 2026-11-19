__all__ = []

from . import v1
__all__.extend(v1.__all__)
from .v1 import *

from . import DetectorConstruction_v1
__all__.extend(DetectorConstruction_v1.__all__)
from .DetectorConstruction_v1 import *

from . import DetectorConstruction_v2
__all__.extend(DetectorConstruction_v2.__all__)
from .DetectorConstruction_v2 import *

# --- Aditivo: utilitario de phi da camara de muons ---
from . import MuonDetectorGeometry
__all__.extend(MuonDetectorGeometry.__all__)
from .MuonDetectorGeometry import *
