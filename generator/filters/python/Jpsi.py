__all__ = ["Jpsi"]

from GaugiKernel import Cpp
from ROOT import generator


class Jpsi(Cpp):

    def __init__(self, name, gen,
                 EtaMax: float = 1.4,
                 MinPt: float = 3.0,
                 ZeroVertexParticles: bool = False,
                 ForceForwardElectron: bool = False,
                 OutputLevel: int = 0
                 ):

        Cpp.__init__(self, generator.Jpsi(name, gen.core()))
        self.__gen = gen

        self.setProperty("EtaMax", EtaMax)
        self.setProperty("MinPt", MinPt)
        self.setProperty("ZeroVertexParticles", ZeroVertexParticles)
       #self.setProperty("ForceForwardElectron", ForceForwardElectron)

    def gun(self):
        return self.__gen

