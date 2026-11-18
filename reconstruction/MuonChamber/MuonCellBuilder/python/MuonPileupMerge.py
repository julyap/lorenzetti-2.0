# ============================================================================
# Arquivo: MuonPileupMerge.py
# Descrição: Wrapper Python para o algoritmo C++ "MuonPileupMerge". Recebe
#            um arquivo de entrada (InputFile) com uma coleção de hits de
#            muon e mistura ("merge") essa coleção com a coleção atual do
#            evento, produzindo novas coleções de hits e de EventInfo
#            (OutputHitsKey e OutputEventKey). Usado para sobrepor eventos
#            de pileup à simulação principal.
# ============================================================================

__all__ = ["MuonPileupMerge"]

# --- Imports do framework e ROOT -------------------------------------------
from GaugiKernel        import Cpp, LoggingLevel
from GaugiKernel.macros import *
import ROOT


class MuonPileupMerge( Cpp ):
    """
    Encapsula o algoritmo C++ MuonPileupMerge, responsável por mesclar
    coleções de hits de muon oriundas de um arquivo externo (pileup).
    """

    def __init__( self, name      : str,
                  InputFile       : str,                          # arquivo de entrada com pileup
                  InputHitsKey    : str="MuonHits",               # coleção de hits atual
                  OutputHitsKey   : str="MuonHits_Merged",        # coleção de hits após merge
                  InputEventKey   : str="EventInfo",              # EventInfo atual
                  OutputEventKey  : str="EventInfo_Merged",       # EventInfo após merge
                  OutputLevel     : int=LoggingLevel.toC('INFO'), # nível de log
                  NtupleName      : str="CollectionTree"          # nome da ntuple dentro do arquivo
                ):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonPileupMerge(name) )

        # --- Configuração das propriedades do algoritmo ----------------
        self.setProperty( "OutputLevel"   , OutputLevel    )   # nível de log
        self.setProperty( "InputHitsKey"  , InputHitsKey   )   # hits de entrada
        self.setProperty( "OutputHitsKey" , OutputHitsKey  )   # hits de saída
        self.setProperty( "InputEventKey" , InputEventKey  )   # EventInfo de entrada
        self.setProperty( "OutputEventKey", OutputEventKey )   # EventInfo de saída
        self.setProperty( "InputFile"     , InputFile      )   # arquivo de pileup
        self.setProperty( "NtupleName"    , NtupleName     )   # nome da ntuple no arquivo
