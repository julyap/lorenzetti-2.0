# ============================================================================
# Arquivo: MuonPulseGenerator.py
# Descrição: Wrapper Python para o algoritmo C++ "MuonPulseGenerator".
#            Responsável por gerar a forma de onda (pulso) das amostras do
#            sinal de uma câmara de muon a partir de um arquivo de shaper
#            (ShaperFile). Permite configurar:
#              - número de amostras (NSamples);
#              - pedestal;
#              - deformação do pulso (média/desvio);
#              - ruído (média/desvio);
#              - taxa de amostragem;
#              - BCID inicial de amostragem.
# ============================================================================

__all__ = ["MuonPulseGenerator"]

# --- Imports do framework e ROOT -------------------------------------------
from GaugiKernel        import Cpp, LoggingLevel
from GaugiKernel.macros import *
import ROOT


class MuonPulseGenerator( Cpp ):
    """
    Encapsula o algoritmo C++ MuonPulseGenerator, responsável por gerar o
    pulso analógico amostrado de uma câmara de muon.
    """

    def __init__( self, name      : str,
                  OutputLevel     : int=LoggingLevel.toC('INFO'), # nível de log
                  NSamples        : int=0,                        # número de amostras
                  ShaperFile      : str="",                       # arquivo do shaper (forma do pulso)
                  Pedestal        : float=0,                      # pedestal do sinal
                  DeformationMean : float=0,                      # deformação média do pulso
                  DeformationStd  : float=0,                      # desvio da deformação
                  NoiseMean       : float=0,                      # média do ruído
                  NoiseStd        : float=0,                      # desvio do ruído
                  SamplingRate    : float=0,                      # taxa de amostragem (ns)
                  StartSamplingBC : float=0                       # BCID inicial de amostragem
                ):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonPulseGenerator(name) )

        # --- Configuração das propriedades do algoritmo ----------------
        self.setProperty( "OutputLevel"     , OutputLevel       )   # nível de log
        self.setProperty( "NSamples"        , NSamples          )   # número de amostras
        self.setProperty( "ShaperFile"      , ShaperFile        )   # arquivo do shaper
        self.setProperty( "Pedestal"        , Pedestal          )   # pedestal
        self.setProperty( "DeformationMean" , DeformationMean   )   # deformação média
        self.setProperty( "DeformationStd"  , DeformationStd    )   # desvio da deformação
        self.setProperty( "NoiseMean"       , NoiseMean         )   # média do ruído
        self.setProperty( "NoiseStd"        , NoiseStd          )   # desvio do ruído
        self.setProperty( "SamplingRate"    , SamplingRate      )   # taxa de amostragem
        self.setProperty( "StartSamplingBC" , StartSamplingBC   )   # BCID inicial
