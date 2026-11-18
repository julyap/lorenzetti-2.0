__all__ = ["PhysicalVolume", "ProductionCuts", "Plates"]

from GaugiKernel import Logger
from GaugiKernel.macros import MSG_FATAL
from GaugiKernel.constants import um
from GaugiKernel import EnumStringification


class Plates(EnumStringification):
  Horizontal = 0
  Vertical   = 1


#
# Geant4 physical volume dimensions
#
class PhysicalVolume(Logger):

    __allow_keys = [
                      "Name",
                      "Plates",
                      "AbsorberMaterial",
                      "GapMaterial",
                      "NofLayers",
                      "AbsorberThickness",
                      "GapThickness",
                      "RMin",
                      "RMax",
                      "ZSize",
                      "X",
                      "Y",
                      "Z",
                      "Visualization",
                      "Color",
                      # --- Campos aditivos da camara de muons (defaults preservam o
                      #     comportamento dos volumes existentes: ECAL/TILE/EMEC/HEC) ---
                      "ChamberStructure",
                      "GasMixture",
                      "StructureThickness",
                      "GasGapThickness",
                      "IsMuonChamber",
                      "ChamberType",
                      "OperatingVoltage",
                      "GasPressure",
                      "MuonCut",
                   ]

    # Constructor
    def __init__(self, **kw):

        Logger.__init__(self)

        # Defaults aditivos de muons (aplicados ANTES do loop, para que kwargs
        # do usuario tenham precedencia; volumes existentes nao passam esses
        # kwargs e ficam com os defaults inertes abaixo)
        self.ChamberStructure  = None
        self.GasMixture        = None
        self.StructureThickness = 0.0
        self.GasGapThickness   = 0.0
        self.IsMuonChamber     = False
        self.ChamberType       = 0
        self.OperatingVoltage  = 0.0
        self.GasPressure       = 1.0
        self.MuonCut           = None

        for key, value in kw.items():
          if key in self.__allow_keys:
            setattr(self, key, value )
          else:
            MSG_FATAL( self, "Property with name %s is not allow for %s object", key, self.__class__.__name__)

        self.ZMin = self.Z - self.ZSize / 2
        self.ZMax = self.Z + self.ZSize / 2
        self.Cuts = ProductionCuts()

    def name(self):
      return self.Name.replace('::','_')


# https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/Simulation/G4Atlas/G4AtlasTools/python/G4PhysicsRegionConfig.py
class ProductionCuts(Logger):
  def __init__(self, GammaCut =  700*um, ElectronCut = 700*um, PositronCut = 700*um, PhotonCut = 700*um):
    self.GammaCut=GammaCut; self.ElectronCut=ElectronCut; self.PositronCut=PositronCut; self.PhotonCut=PhotonCut
