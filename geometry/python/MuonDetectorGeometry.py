__all__ = ["MuonDetectorGeometry"]

from GaugiKernel import Logger
from GaugiKernel.macros import *


class MuonDetectorGeometry( Logger ):

  def __init__( self ):     
    Logger.__init__(self)
    import ROOT
    ROOT.gSystem.Load('liblorenzetti')
    from ROOT import MuonDetectorGeometry as MuonDetectorGeometryCore
    # Create the algorithm
    self.__core = MuonDetectorGeometryCore()

  def core(self):
    return self.__core

  def diff(self, phi1, phi2):
    return self.core().diff(phi1, phi2)
  
  def fix(self, phi):
    return self.core().fix(phi)
  
  def twopi(self):
    return self.core().twopi()
  
  def phi_min(self):
    return self.core().phi_min()
  
  def phi_max(self):
    return self.core().phi_max()
