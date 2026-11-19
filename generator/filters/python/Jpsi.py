__all__ = ["Jpsi", "Jpsi_eemumu"]  # <-- ADICIONAR Jpsi_eemumu

from GaugiKernel import Cpp
from ROOT import generator


class Jpsi(Cpp):
    """Classe base para geração de J/ψ - processa ambos os canais (μ⁺μ⁻ e e⁺e⁻)"""

    def __init__(self, name, gen,
                 EtaMax: float = 2.7,
                 MinPt: float = 0.5,  # <-- MUDADO DE 5.0 PARA 0.5
                 ZeroVertexParticles: bool = False,
                 ForceForwardElectron: bool = False,
                 OutputLevel: int = 0
                 ):

        Cpp.__init__(self, generator.Jpsi(name, gen.core()))
        self.__gen = gen

        self.setProperty("EtaMax", EtaMax)
        self.setProperty("MinPt", MinPt)
        self.setProperty("ZeroVertexParticles", ZeroVertexParticles)
        # self.setProperty("ForceForwardElectron", ForceForwardElectron)

    def gun(self):
        return self.__gen


class Jpsi_eemumu(Cpp):
    """Versão estendida que processa J/ψ -> e⁺e⁻ e J/ψ -> μ⁺μ⁻ separadamente"""
    
    def __init__(self, name, gen,
                 EtaMax: float = 2.7,
                 MinPt: float = 0.5,
                 ZeroVertexParticles: bool = False,
                 OutputLevel: int = 0
                 ):
        Cpp.__init__(self, generator.Jpsi_eemumu(name, gen.core()))
        self.__gen = gen

        self.setProperty("EtaMax", EtaMax)
        self.setProperty("MinPt", MinPt)
        self.setProperty("ZeroVertexParticles", ZeroVertexParticles)

    def gun(self):
        return self.__gen
