__all__ = ["CaloClusterMaker"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
from CaloClusterBuilder import CaloClusterFlags as flags
import ROOT

class CaloClusterMaker(Cpp):

  def __init__(self, name,
               InputCellsKey: str, 
               InputSeedsKey: str,
               OutputClusterKey: str, 
               EtaWindow: float = flags.EtaWindow, 
               PhiWindow: float = flags.PhiWindow,
               MinCenterEnergy: float = flags.MinCenterEnergy,
               doForwardMoments: bool = flags.doForwardMoments,
               OutputLevel: str = "0", 
               HistogramPath: str = "Expert/Clusters"):

    cpp_obj = ROOT.CaloClusterMaker(name)
    super().__init__(cpp_obj)

    # Define helper for safe property setting
    def safe_set(key, value):
      try:
        self.setProperty(key, value)
      except Exception:
        raise AttributeError(f"[CaloClusterMaker] Property '{key}' is not valid for this object.")

    safe_set("InputCellsKey",        InputCellsKey)
    safe_set("InputSeedsKey",        InputSeedsKey)
    safe_set("OutputClusterKey",     OutputClusterKey)
    safe_set("EtaWindow",            EtaWindow)
    safe_set("PhiWindow",            PhiWindow)
    safe_set("MinCenterEnergy",      MinCenterEnergy)
    safe_set("DoForwardMoments",     doForwardMoments)
    safe_set("OutputLevel",          OutputLevel)
    safe_set("HistogramPath",        HistogramPath)

