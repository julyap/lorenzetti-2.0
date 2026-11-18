# ============================================================================
# Arquivo: MuonClusterMaker.py
# Autora: Julya Pacheco Ferreira
# Descrição: Wrapper Python para o algoritmo C++ "MuonClusterMaker".
#            Responsável por agrupar ("clusterizar") células de muon em
#            torno de sementes (seeds) com base em janelas de proximidade:
#              - janelas em eta/phi (EtaWindow, PhiWindow);
#              - janelas espaciais em X/Y/Z (XWindow, YWindow, ZWindow);
#              - energia mínima da semente (MinCenterEnergy);
#              - opcionalmente, cálculo de momentos direcionais
#                (DoForwardMoments);
#              - opcionalmente, uma ShapesTool para caracterizar a forma
#                do cluster.
#            Lê a coleção de células (InputCellsKey) e a coleção de
#            sementes (InputSeedsKey), e produz a coleção de clusters
#            (OutputClusterKey).
#            Os valores default das janelas e da energia mínima são lidos
#            de MuonClusterFlags (MuonClusterBuilder.MuonClusterFlags).
# ============================================================================

__all__ = ["MuonClusterMaker"]

# --- Imports do framework e do projeto -------------------------------------
from GaugiKernel import Cpp
from GaugiKernel.macros import *
from MuonClusterBuilder import MuonClusterFlags as flags
import ROOT


class MuonClusterMaker( Cpp ):
    """
    Encapsula o algoritmo C++ MuonClusterMaker, configurando-o a partir
    dos parâmetros Python.
    """

    def __init__( self, name,
                  InputCellsKey    : str = "MuonCells",                 # coleção de células
                  InputSeedsKey    : str = "Seeds",                     # coleção de sementes
                  OutputClusterKey : str = "MuonClusters",              # coleção de saída
                  EtaWindow        : float = flags.EtaWindow,           # janela em eta
                  PhiWindow        : float = flags.PhiWindow,           # janela em phi
                  XWindow          : float = flags.XWindow,             # janela em X
                  YWindow          : float = flags.YWindow,             # janela em Y
                  ZWindow          : float = flags.ZWindow,             # janela em Z
                  MinCenterEnergy  : float = flags.MinCenterEnergy,     # energia mínima da semente
                  DoForwardMoments : bool = flags.DoForwardMoments,     
                  HistPath         : str = "MuonClusterMaker",          # caminho dos histogramas
                  OutputLevel      : int = 0,                           # nível de log
                  ShapesTool       = None,                              # <-- NOVO PARÂMETRO
                ):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonClusterMaker(name) )

        # Set only properties that exist in C++
        self.setProperty("InputCellsKey"    , InputCellsKey    )   # coleção de células
        self.setProperty("InputSeedsKey"    , InputSeedsKey    )   # coleção de sementes
        self.setProperty("OutputClusterKey" , OutputClusterKey )   # coleção de clusters
        self.setProperty("EtaWindow"        , EtaWindow        )   # janela em eta
        self.setProperty("PhiWindow"        , PhiWindow        )   # janela em phi
        self.setProperty("XWindow"          , XWindow          )   # janela em X
        self.setProperty("YWindow"          , YWindow          )   # janela em Y
        self.setProperty("ZWindow"          , ZWindow          )   # janela em Z
        self.setProperty("MinCenterEnergy"  , MinCenterEnergy  )   # energia mínima
        self.setProperty("DoForwardMoments" , DoForwardMoments )   
        self.setProperty("HistPath"         , HistPath         )   # caminho dos histogramas
        self.setProperty("OutputLevel"      , OutputLevel      )   # nível de log

        # <-- NOVO: Set the shapes tool if provided
        # Acopla a ferramenta de caracterização de forma (se fornecida)
        if ShapesTool is not None:
            self.setProperty("ShapesTool", ShapesTool)
