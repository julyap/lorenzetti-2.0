# ============================================================================
# Arquivo: MuonFlags.py
# Descrição: Define as flags globais de configuração para a simulação e
#            reconstrução de muon. Herda de EnumStringification do GaugiKernel
#            e expõe valores default de ruído, cross-talk (energia mínima e
#            amplitudes capacitivas/indutivas/resistivas). Essas flags são
#            usadas como valores default por outros componentes (e.g.
#            MuonCrossTalkMaker).
# ============================================================================

__all__ = ["MuonFlags"]


# --- Imports do framework ---------------------------------------------------
from GaugiKernel import EnumStringification
from GaugiKernel.constants import *


class MuonFlags(EnumStringification):
    """
    Conjunto de flags/configurações globais para o subsistema de muon.
    """

    # hit global parameters
    NoiseStd            = 0.0           # desvio padrão do ruído global (default: sem ruído)

    # crosstalk flags (typically not used for muon chambers)
    DoCrossTalk         = False         # liga/desliga cross-talk (default: desligado)
    XTMinEnergy         = 1*GeV         # energia mínima para aplicar cross-talk
    XTAmpCapacitive     = 4.2           # amplitude default do cross-talk capacitivo
    XTAmpInductive      = 2.3           # amplitude default do cross-talk indutivo
    XTAmpResistive      = 1.0           # amplitude default do cross-talk resistivo
