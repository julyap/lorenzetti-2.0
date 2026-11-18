# ============================================================================
# Arquivo: MuonOptimalFilter.py
# Descrição: Wrappers Python para dois algoritmos C++ de filtro ótimo usados
#            na reconstrução de muon:
#
#              1) MuonOptimalFilter          - filtro ótimo simples, que aplica
#                                              pesos de energia e de tempo
#                                              (WeightsEnergy/WeightsTime) à
#                                              amostragem do pulso.
#              2) MuonConstrainedOptimalFilter - variante restrita, que recebe
#                                              parâmetros do pulso (caminho,
#                                              threshold, taxa de amostragem,
#                                              número de amostras etc.).
# ============================================================================

__all__ = ["MuonOptimalFilter", "MuonConstrainedOptimalFilter"]


# --- Imports do framework e ROOT -------------------------------------------
from GaugiKernel        import Cpp, LoggingLevel
from GaugiKernel.macros import *
import ROOT


class MuonOptimalFilter( Cpp ):
    """
    Encapsula o algoritmo C++ MuonOptimalFilter (filtro ótimo simples).
    """

    def __init__( self, name    : str,
                  WeightsEnergy : list=[],                       # pesos de energia por amostra
                  WeightsTime   : list=[],                       # pesos de tempo por amostra
                  OutputLevel   : int=LoggingLevel.toC('INFO'),  # nível de log
                  MDTCalibrationFactor: float = 0.001,           # NOVO PARÂMETRO
                ):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonOptimalFilter(name) )

        # --- Configuração das propriedades do algoritmo ----------------
        self.setProperty( "OutputLevel"   , OutputLevel   )   # nível de log
        self.setProperty( "WeightsEnergy" , WeightsEnergy )   # pesos de energia
        self.setProperty( "WeightsTime"   , WeightsTime   )   # pesos de tempo

        # Guardar para uso futuro (não é passado via setProperty)
        self.MDTCalibrationFactor = MDTCalibrationFactor


class MuonConstrainedOptimalFilter(Cpp):
    """
    Encapsula o algoritmo C++ MuonConstrainedOptimalFilter (filtro ótimo
    com restrições baseadas na forma do pulso).
    """

    def __init__( self, name,
                  OutputLevel       : int=0,     # nível de log
                  NSamples          : float=0,   # número de amostras
                  PulsePath         : str="",    # caminho do arquivo de pulso
                  Threshold         : float=0,   # threshold aplicado
                  SamplingRate      : float=0,   # taxa de amostragem (ns)
                  StartSamplingBC   : float=0,   # BCID inicial de amostragem
                ):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonConstrainedOptimalFilter(name) )

        # --- Configuração das propriedades do algoritmo ----------------
        self.setProperty( "OutputLevel"     , OutputLevel       )   # nível de log
        self.setProperty( "OutputLevel"     , OutputLevel       )   # (duplicado no original — mantido como está)
        self.setProperty( "NSamples"        , NSamples          )   # número de amostras
        self.setProperty( "PulsePath"       , PulsePath         )   # arquivo de pulso
        self.setProperty( "Threshold"       , Threshold         )   # threshold
        self.setProperty( "SamplingRate"    , SamplingRate      )   # taxa de amostragem
        self.setProperty( "StartSamplingBC" , StartSamplingBC   )   # BCID inicial
