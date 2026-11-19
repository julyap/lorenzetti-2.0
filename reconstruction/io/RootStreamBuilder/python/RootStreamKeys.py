__all__ = ["recordable"]

valid_keys = [
    # --- Oficial (todas as 27 chaves preservadas) ---
    "EventInfo",
    "Hits",
    # CaloCellMerge
    "Cells",
    "TruthCells",
    # CaloClusterMaker
    "Events",
    "Seeds",
    "Clusters",
    "TruthClusters",
    "Particles",
    "TruthParticles",
    # CaloRingsBuilder
    "Rings",
    "RingsSigmaNoise",
    "TruthRings",
    "AsymRings",
    "TruthAsymRings",
    "CornerRings",
    "TruthCornerRings",
    "StripsRings",
    "TruthStripsRings",
    "CrossRings",
    "TruthCrossRings",
    "CustomRings",
    "TruthCustomRings",
    "Electrons",
    "TruthElectrons",
    "RingsL0",
    "TruthRingsL0",
    # --- Aditivo: chaves da camara de muons ---
    "Event",
    "MuonHits",
    "CaloHits",
    "MuonHits_Processed",
    "TruthMuonHits",
    "CaloCells",
    "TruthCaloCells",
    "MuonCells",
    "TruthMuonCells",
    "MuonClusters",
    "Muons",
]


def recordable(key):
    if key not in valid_keys:
        raise RuntimeError(f"Key {key} not allow to be recorded.")
    return key
