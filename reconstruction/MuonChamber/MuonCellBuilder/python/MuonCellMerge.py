# ============================================================================
# Arquivo: MuonCellMerge.py
# Descrição: Wrapper Python para o algoritmo C++ "MuonCellMerge". Recebe
#            uma lista de chaves de coleções de células de muon (produzidas
#            por cada MuonCellMaker) e as funde em uma única coleção de
#            saída (OutputCellsKey) e em uma coleção de células de verdade
#            (OutputTruthCellsKey).
# ============================================================================

__all__ = ["MuonCellMerge"]

# --- Imports do framework e ROOT -------------------------------------------
from GaugiKernel        import Cpp, LoggingLevel
from GaugiKernel.macros import *
import ROOT


class MuonCellMerge( Cpp ):
    """
    Encapsula o algoritmo C++ MuonCellMerge, responsável por juntar as
    coleções de células produzidas individualmente em uma única coleção.
    """

    def __init__( self, name          : str,
                  InputCollectionKeys : str="Collection",
                  OutputCellsKey      : str="MuonCells",
                  OutputTruthCellsKey : str="TruthMuonCells",
                  OutputLevel         : int=LoggingLevel.toC('INFO'),
                  ):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonCellMerge(name) )

        # --- Configuração das propriedades do algoritmo ----------------
        self.setProperty( "InputCollectionKeys" , InputCollectionKeys )   # lista de coleções a fundir
        self.setProperty( "OutputCellsKey"      , OutputCellsKey      )   # coleção final de células
        self.setProperty( "OutputTruthCellsKey" , OutputTruthCellsKey )   # coleção final de truth cells
        self.setProperty( "OutputLevel"         , OutputLevel         )   # nível de log
