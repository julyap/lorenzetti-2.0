# ============================================================================
# Arquivo: MuonCrossTalkMaker.py
# Descrição: Wrapper Python para o algoritmo C++ "MuonCrossTalkMaker".
#            Simula o efeito de cross-talk (capacitivo, indutivo e resistivo)
#            entre hits de muon, aplicando amplitudes de cross-talk sobre a
#            coleção de hits de entrada e produzindo uma nova coleção de
#            hits com o cross-talk aplicado. Parâmetros default (energia
#            mínima e amplitudes) são lidos das flags globais em MuonFlags.
# ============================================================================

__all__ = ["MuonCrossTalkMaker"]

# --- Imports do framework e ROOT -------------------------------------------
from GaugiKernel        import Cpp, LoggingLevel
from GaugiKernel.macros import *
from MuonHitBuilder import MuonFlags as flags   # flags globais de muon (defaults)
import ROOT


class MuonCrossTalkMaker( Cpp ):
    """
    Encapsula o algoritmo C++ MuonCrossTalkMaker, responsável por aplicar
    cross-talk sobre os hits de muon.
    """

    def __init__( self, name          : str,
                  InputCollectionKey  : str="MuonHits",
                  OutputCollectionKey : str="XTMuonHits",
                  MinEnergy           : float=flags.XTMinEnergy,       # energia mínima para aplicar XT
                  XTAmpCapacitive     : float=flags.XTAmpCapacitive,   # amplitude do XT capacitivo
                  XTAmpInductive      : float=flags.XTAmpInductive,    # amplitude do XT indutivo
                  XTAmpResistive      : float=flags.XTAmpResistive,    # amplitude do XT resistivo
                  HistogramPath       : str="/MuonCrossTalkMaker",
                  OutputLevel         : int=LoggingLevel.toC('INFO'),
                  ):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonCrossTalkMaker(name) )

        # Lista de ferramentas acopláveis (Tools)
        self.Tools = []

        # --- Configuração das propriedades do algoritmo ----------------
        self.setProperty( "OutputLevel"           , OutputLevel         )   # nível de log
        self.setProperty( "InputCollectionKey"    , InputCollectionKey  )   # coleção de hits de entrada
        self.setProperty( "OutputCollectionKey"   , OutputCollectionKey )   # coleção de hits com XT
        self.setProperty( "MinEnergy"             , MinEnergy           )   # energia mínima
        self.setProperty( "XTAmpCapacitive"       , XTAmpCapacitive     )   # amplitude capacitiva
        self.setProperty( "XTAmpInductive"        , XTAmpInductive      )   # amplitude indutiva
        self.setProperty( "XTAmpResistive"        , XTAmpResistive      )   # amplitude resistiva
        self.setProperty( "HistogramPath"         , HistogramPath       )   # caminho dos histogramas


    def core(self):
        """Anexa todas as Tools ao core C++ antes de retorná-lo."""
        # Attach all tools before return the core
        for tool in self.Tools:
            self._core.push_back(tool.core())
        return self._core


    def __add__( self, tool ):
        """Permite usar o operador '+' para acoplar ferramentas (Tools)."""
        self.Tools += tool
        return self
