# ============================================================================
# Arquivo: MuonClusterFlags.py
# Autora: Julya Pacheco Ferreira
# Descrição: Define as flags globais de configuração para o agrupamento
#            (clustering) de células de muon. Herda de EnumStringification
#            do GaugiKernel e expõe valores default para:
#              - energia mínima da semente (MinCenterEnergy);
#              - janelas de agrupamento geométrico em eta/phi e em X/Y/Z;
#              - janela temporal (TimeWindow);
#              - flags de processamento (DoForwardMoments, DoCurvatureCalc,
#                DoTimeAnalysis, DoLayerEfficiency).
#            Os valores default são baseados nos parâmetros otimizados para
#            muons do ATLAS 2016 (Eur. Phys. J. C 76 (2016) 292,
#            arXiv:1603.05598).
# ============================================================================

__all__ = ["MuonClusterFlags"]


# --- Imports do framework ---------------------------------------------------
from GaugiKernel import EnumStringification
from GaugiKernel.constants import *


class MuonClusterFlags(EnumStringification):
    """
    Conjunto de flags/configurações globais para o subsistema de clustering
    de muon.
    """

    # ============================================================
    # PARÂMETROS OTIMIZADOS PARA MUONS (ATLAS 2016)
    # Baseado em: Eur. Phys. J. C 76 (2016) 292, arXiv:1603.05598
    # ============================================================

    # Energia mínima para considerar uma célula como semente
    # ATLAS: muons depositam ~0.1-10 GeV (MIPs)
    MinCenterEnergy   = 0.01*GeV     # 10 MeV - reduzido para capturar MIPs

    # Janelas de agrupamento geométrico
    # ATLAS: muons têm trajetórias retilíneas, janelas pequenas
    EtaWindow         = 0.1          # Δη pequeno para múons
    PhiWindow         = 0.1          # Δφ pequeno para múons

    # Janelas espaciais (mm)
    XWindow           = 500.0        # mm
    YWindow           = 500.0        # mm
    ZWindow           = 500.0        # mm

    # Janela temporal para agrupamento
    TimeWindow        = 10.0*ns      # ns

    # ============================================================
    # FLAGS DE PROCESSAMENTO (ATLAS 2016)
    # ============================================================

    # Calcular qualidade do muon (ATLAS página 6-7)
    # Usa: chi2, q/p significance, rho prime
    DoForwardMoments  = True         # Habilitado para calcular qualidade

    # Calcular curvatura da trajetória (útil para identificar kinks)
    DoCurvatureCalc   = True

    # Análise de tempo do cluster
    DoTimeAnalysis    = True

    # Cálculo de eficiência por camada
    DoLayerEfficiency = True
