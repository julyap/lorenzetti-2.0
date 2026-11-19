__all__ = ["RootStreamNtupleMaker"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
import ROOT


class RootStreamNtupleMaker(Cpp):

    def __init__(
        self,
        name,
        InputEventKey: str,
        InputTruthKey: str,
        InputSeedsKey: str,
        # --- Oficial: calorimetro ---
        InputClusterKey: str = None,
        InputRingerKeys: str = None,
        InputRingerKey: str = None,          # alias
        InputRingerL0Key: str = None,
        InputTruthClusterKey: str = None,
        InputTruthRingerKeys: str = None,
        InputTruthElectronKey: str = None,
        InputElectronKey: str = None,
        # --- Aditivo: muon ---
        InputMuonKey: str = None,
        InputMuonClusterKey: str = None,
        # --- Oficial ---
        OutputLevel: int = 0,
        OutputNtupleName: str = "events",
        # --- Aditivo: flags (default preservam o comportamento oficial) ---
        EnableCaloOutput: bool = True,
        EnableMuonOutput: bool = True,
    ):

        try:
            Cpp.__init__(self, ROOT.Gaugi.RootStreamNtupleMaker(name))
        except:
            Cpp.__init__(self, ROOT.RootStreamNtupleMaker(name))

        _ringer = InputRingerKey if InputRingerKey is not None else InputRingerKeys

        # Comuns (sempre presentes)
        self.setProperty("InputEventKey", InputEventKey)
        self.setProperty("InputTruthKey", InputTruthKey)
        self.setProperty("InputSeedsKey", InputSeedsKey)

        # Calorimeter (oficial)
        if InputClusterKey:
            self.setProperty("InputClusterKey", InputClusterKey)
        if _ringer:
            self.setProperty("InputRingerKeys", _ringer)
        if InputRingerL0Key:
            self.setProperty("InputRingerL0Key", InputRingerL0Key)
        if InputTruthClusterKey:
            self.setProperty("InputTruthClusterKey", InputTruthClusterKey)
        if InputTruthRingerKeys:
            self.setProperty("InputTruthRingerKeys", InputTruthRingerKeys)
        if InputTruthElectronKey:
            self.setProperty("InputTruthElectronKey", InputTruthElectronKey)
        if InputElectronKey:
            self.setProperty("InputElectronKey", InputElectronKey)

        # Muon (aditivo)
        if EnableMuonOutput:
            if InputMuonKey:
                self.setProperty("InputMuonKey", InputMuonKey)
            if InputMuonClusterKey:
                self.setProperty("InputMuonClusterKey", InputMuonClusterKey)

        # Output (oficial)
        self.setProperty("OutputNtupleName", OutputNtupleName)
        self.setProperty("OutputLevel", OutputLevel)

        # Flags (aditivo)
        self.setProperty("EnableCaloOutput", EnableCaloOutput)
        self.setProperty("EnableMuonOutput", EnableMuonOutput)
