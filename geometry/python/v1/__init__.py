__all__ = []

from . import PhysicalVolume
__all__.extend(PhysicalVolume.__all__)
from .PhysicalVolume import *

from . import SensitiveDetector
__all__.extend(SensitiveDetector.__all__)
from .SensitiveDetector import *

from . import Calorimeter
__all__.extend(Calorimeter.__all__)
from .Calorimeter import *

from . import Tracking
__all__.extend(Tracking.__all__)
from .Tracking import *

from . import ECAL
__all__.extend(ECAL.__all__)
from .ECAL import *

from . import TILE
__all__.extend(TILE.__all__)
from .TILE import *

from . import EMEC
__all__.extend(EMEC.__all__)
from .EMEC import *

from . import HEC
__all__.extend(HEC.__all__)
from .HEC import *

from . import DeadMaterials
__all__.extend(DeadMaterials.__all__)
from .DeadMaterials import *

# Muon chambers (configuracao Python; dependem de PhysicalVolume estendida,
# SensitiveDetector com alias SensitiveCaloVolume e dos modulos MuonChamber/MuonEndcap
# que devem morar em geometry/python/v1/)
try:
    from . import MuonChamber
    __all__.extend(MuonChamber.__all__)
    from .MuonChamber import *
except Exception as _e:
    import warnings
    warnings.warn(f"MuonChamber nao carregado: {_e}")

try:
    from . import MuonEndcap
    __all__.extend(MuonEndcap.__all__)
    from .MuonEndcap import *
except Exception as _e:
    import warnings
    warnings.warn(f"MuonEndcap nao carregado: {_e}")
