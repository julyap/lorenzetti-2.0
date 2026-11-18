# ============================================================================
# Arquivo: MuonMaker.py
# Autora: Julya Pacheco Ferreira
# Descrição: Wrapper Python para o algoritmo C++ "MuonMaker". Recebe a
#            coleção de clusters de múons (InputMuonClusterKey) e produz a
#            coleção de múons reconstruídos (OutputMuonKey), aplicando
#            cortes de qualidade configuráveis para quatro categorias:
#              - VLooseCuts, LooseCuts, MediumCuts, TightCuts.
#            Cada conjunto de cortes é passado no formato
#            [chi2_cut, min_cells, min_fracMax].
#            NOTA: TightCuts corresponde a uma aproximação "Tight-like" para
#            múons MS-only, NÃO ao Tight oficial do ATLAS (que exige
#            combinação ID+MS).
# ============================================================================

__all__ = ["MuonMaker"]

# --- Imports do framework e utilitários ------------------------------------
from GaugiKernel import Cpp
from typing import List
import ROOT


class MuonMaker(Cpp):
    def __init__(self, name,
                 InputMuonClusterKey: str,
                 OutputMuonKey: str,
                 OutputLevel: int = 0,
                 TightCuts: List[float] = [],     # [chi2_cut, min_cells, min_fracMax] — Tight-like (MS-only)
                 MediumCuts: List[float] = [],    # [chi2_cut, min_cells, min_fracMax]
                 LooseCuts: List[float] = [],     # [chi2_cut, min_cells, min_fracMax]
                 VLooseCuts: List[float] = []):   # [chi2_cut, min_cells, min_fracMax]

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonMaker(name))

        # --- Configuração das propriedades do algoritmo ----------------
        self.setProperty("InputMuonClusterKey", InputMuonClusterKey)   # coleção de clusters de entrada
        self.setProperty("OutputMuonKey", OutputMuonKey)               # coleção de múons de saída
        self.setProperty("OutputLevel", OutputLevel)                   # nível de log
        self.setProperty("TightCuts", TightCuts)                       # cortes Tight-like (MS-only)
        self.setProperty("MediumCuts", MediumCuts)                     # cortes da categoria Medium
        self.setProperty("LooseCuts", LooseCuts)                       # cortes da categoria Loose
        self.setProperty("VLooseCuts", VLooseCuts)                     # cortes da categoria Very Loose
