# ============================================================================
# Arquivo: MuonHitMerge.py
# Descrição: Wrapper Python para o algoritmo C++ "MuonHitMerge". Recebe uma
#            lista de chaves de coleções de hits de muon (produzidas por cada
#            MuonHitMaker) e as funde em uma única coleção de hits de saída
#            (OutputHitsKey).
# ============================================================================

__all__ = ["MuonHitMerge"]

# --- Imports do framework e ROOT -------------------------------------------
from GaugiKernel        import Cpp, LoggingLevel
from GaugiKernel.macros import *
import ROOT


class MuonHitMerge( Cpp ):
    """
    Encapsula o algoritmo C++ MuonHitMerge, responsável por juntar as
    coleções de hits produzidas individualmente em uma única coleção.
    """

    def __init__( self, name          : str,
                  InputCollectionKeys : list= [],                       # lista de coleções a fundir
                  OutputHitsKey       : str = "MuonHits",               # coleção final de hits
                  OutputLevel         : int = LoggingLevel.toC('INFO'), # nível de log
                  ):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonHitMerge(name) )

        # --- Configuração das propriedades do algoritmo ----------------
        self.setProperty( "InputCollectionKeys" , InputCollectionKeys )   # coleções de entrada
        self.setProperty( "OutputHitsKey"       , OutputHitsKey       )   # coleção de saída
        self.setProperty( "OutputLevel"         , OutputLevel         )   # nível de log
