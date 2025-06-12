___all__ = ["Upsilon"]

from GaugiKernel import Cpp
from ROOT import generator


class Upsilon(Cpp):

    def __init__(self, name, gen,
                 EtaMax: float = 1.4,
                 MinMass: float = 9.0,
                 MinPt: float = 15.0,
                 ZeroVertexParticles: bool = False,
      #           ForceForwardElectron: bool = False,
                 OutputLevel: int = 0
                 ):

        Cpp.__init__(self, generator.Upsilon(name, gen.core()))
        self.__gen = gen

        self.setProperty("EtaMax", EtaMax)
        self.setProperty("MinPt", MinPt)
        self.setProperty("ZeroVertexParticles", ZeroVertexParticles)
       # self.setProperty("ForceForwardElectron", ForceForwardElectron)

    def gun(self):
        return self.__gen

