
__all__ = ["RootStreamAODReader"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
import ROOT


class RootStreamAODReader(Cpp):

    def __init__(
        self,
        name,
        InputFile: str,
        OutputEventKey: str,
        OutputTruthKey: str,
        OutputSeedsKey: str,

        # Calorímetro
        OutputClusterKey: str = None,
        OutputRingerKeys: list = None,
        OutputRingerL0Key: str = None,
        OutputElectronKey: str = None,

        # Truth
        OutputTruthClusterKey: str = None,
        OutputTruthRingerKeys: list = None,
        OutputTruthElectronKey: str = None,

        # Sistema de múons
        OutputMuonKey: str = None,
        OutputMuonClusterKey: str = None,
        OutputMuonCellsKey: str = None,
        OutputMuonDescriptorKey: str = None,

        # Configuração
        OutputLevel: int = 0,
        NtupleName: str = "CollectionTree",

        # Flags
        EnableCaloOutput: bool = True,
        EnableMuonOutput: bool = True,
        EnableMuonCellsOutput: bool = True,
    ):

        Cpp.__init__(self, ROOT.RootStreamAODReader(name))

        # ---------------------------------------------------------
        # Propriedades obrigatórias
        # ---------------------------------------------------------
        self.setProperty("OutputEventKey", OutputEventKey)
        self.setProperty("OutputTruthKey", OutputTruthKey)
        self.setProperty("OutputSeedsKey", OutputSeedsKey)

        self.setProperty("OutputLevel", OutputLevel)
        self.setProperty("NtupleName", NtupleName)
        self.setProperty("InputFile", InputFile)

        # ---------------------------------------------------------
        # Calorímetro
        # ---------------------------------------------------------
        if OutputClusterKey is not None:
            self.setProperty("OutputClusterKey", OutputClusterKey)

        if OutputRingerKeys is not None:
            self.setProperty("OutputRingerKeys", OutputRingerKeys)

        if OutputRingerL0Key is not None:
            self.setProperty("OutputRingerL0Key", OutputRingerL0Key)

        if OutputElectronKey is not None:
            self.setProperty("OutputElectronKey", OutputElectronKey)

        # ---------------------------------------------------------
        # Truth
        # ---------------------------------------------------------
        if OutputTruthClusterKey is not None:
            self.setProperty("OutputTruthClusterKey", OutputTruthClusterKey)

        if OutputTruthRingerKeys is not None:
            self.setProperty("OutputTruthRingerKeys", OutputTruthRingerKeys)

        if OutputTruthElectronKey is not None:
            self.setProperty("OutputTruthElectronKey", OutputTruthElectronKey)

        # ---------------------------------------------------------
        # Sistema de múons
        # ---------------------------------------------------------
        if OutputMuonKey is not None:
            self.setProperty("OutputMuonKey", OutputMuonKey)

        if OutputMuonClusterKey is not None:
            self.setProperty("OutputMuonClusterKey", OutputMuonClusterKey)

        if OutputMuonCellsKey is not None:
            self.setProperty("OutputMuonCellsKey", OutputMuonCellsKey)

        if OutputMuonDescriptorKey is not None:
            self.setProperty("OutputMuonDescriptorKey", OutputMuonDescriptorKey)

        # ---------------------------------------------------------
        # Flags
        # ---------------------------------------------------------
        self.setProperty("EnableCaloOutput", EnableCaloOutput)
        self.setProperty("EnableMuonOutput", EnableMuonOutput)
        self.setProperty("EnableMuonCellsOutput", EnableMuonCellsOutput)

        # ---------------------------------------------------------
        # Número de eventos
        # ---------------------------------------------------------
        f = ROOT.TFile(self.InputFile, "READ")

        if not f or f.IsZombie():
            raise RuntimeError(
                f"Não foi possível abrir o arquivo AOD: {self.InputFile}"
            )

        t = f.Get(self.NtupleName)

        if not t:
            f.Close()
            raise RuntimeError(
                f"Não foi possível encontrar a árvore {self.NtupleName} "
                f"no arquivo {self.InputFile}"
            )

        self.__entries = t.GetEntries()

        f.Close()

    def GetEntries(self):
        return self.__entries

    def merge(self, acc):
        acc.SetReader(self)

