# ============================================================================
# Arquivo: MuonDefs.py
# Autora: Julya Pacheco Ferreira
# Descrição: Define as enumerações (EnumStringification) usadas para
#            identificar os tipos de câmara, estações, camadas e regiões do
#            Espectrômetro de Múons do ATLAS. Essas enumerações são usadas
#            consistentemente em toda a cadeia de simulação/reconstrução
#            (geometria, hits, células, clusters, múons) para evitar
#            ambiguidade entre nomes e códigos inteiros.
# ============================================================================

__all__ = ["MuonChamberType", "MuonStation", "MuonLayer", "MuonRegion"]

from GaugiKernel import EnumStringification


class MuonChamberType(EnumStringification):
    """Tipos de câmara de múon do ATLAS (códigos inteiros)."""
    MDT  = 0  # Monitored Drift Tubes
    RPC  = 1  # Resistive Plate Chambers
    CSC  = 2  # Cathode Strip Chambers
    TGC  = 3  # Thin Gap Chambers
    sTGC = 4  # small Thin Gap Chambers
    MM   = 5  # MicroMegas


class MuonStation(EnumStringification):
    """Estações (stations) do Espectrômetro de Múons do ATLAS."""
    BIL  = 0   # Barrel Inner Large
    BIS  = 1   # Barrel Inner Small
    BML  = 2   # Barrel Middle Large
    BMS  = 3   # Barrel Middle Small
    BOL  = 4   # Barrel Outer Large
    BOS  = 5   # Barrel Outer Small
    BEE  = 6   # Barrel Endcap Extended
    EIL  = 7   # Endcap Inner Large
    EIS  = 8   # Endcap Inner Small
    EML  = 9   # Endcap Middle Large
    EMS  = 10  # Endcap Middle Small
    EOL  = 11  # Endcap Outer Large
    EOS  = 12  # Endcap Outer Small
    EEL  = 13  # Endcap Extended Large
    EES  = 14  # Endcap Extended Small
    CSC1 = 15  # Cathode Strip Chamber Type 1
    CSC2 = 16  # Cathode Strip Chamber Type 2


class MuonLayer(EnumStringification):
    """Camadas (layers) dentro de uma câmara de múon."""
    InnerLayer  = 0
    MiddleLayer = 1
    OuterLayer  = 2
    ExtraLayer  = 3


class MuonRegion(EnumStringification):
    """Regiões geométricas do Espectrômetro de Múons."""
    Barrel         = 0
    Endcap         = 1
    Forward        = 2
    ExtendedBarrel = 3
