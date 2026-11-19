__all__ = ["RootStreamAODMaker"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
from RootStreamBuilder import RootStreamAODFlags as flags
import ROOT


class RootStreamAODMaker(Cpp):

    def __init__(
        self,
        name,

        # ==================================================
        # ENTRADAS COMUNS
        # ==================================================
        InputEventKey: str,
        InputTruthKey: str,
        InputSeedsKey: str,

        # ==================================================
        # ENTRADAS DO CALORÍMETRO
        # ==================================================
        InputCellsKey: str = None,
        InputTruthCellsKey: str = None,
        InputClusterKey: str = None,
        InputTruthClusterKey: str = None,
        InputRingerKeys: list = None,
        InputTruthRingerKeys: list = None,
        InputElectronKey: str = None,
        InputTruthElectronKey: str = None,
        InputRingerL0Key: str = None,

        # ==================================================
        # ENTRADAS DA CÂMARA DE MÚONS
        # ==================================================
        InputMuonKey: str = None,
        InputMuonCellsKey: str = None,
        InputMuonClusterKey: str = None,

        # ==================================================
        # SAÍDAS COMUNS
        # ==================================================
        OutputEventKey: str = None,
        OutputTruthKey: str = None,
        OutputSeedsKey: str = None,

        # ==================================================
        # SAÍDAS DO CALORÍMETRO
        # ==================================================
        OutputCellsKey: str = None,
        OutputTruthCellsKey: str = None,
        OutputClusterKey: str = None,
        OutputTruthClusterKey: str = None,
        OutputRingerKeys: list = None,
        OutputTruthRingerKeys: list = None,
        OutputElectronKey: str = None,
        OutputTruthElectronKey: str = None,
        OutputRingerL0Key: str = None,

        # ==================================================
        # SAÍDAS DA CÂMARA DE MÚONS
        # ==================================================
        OutputMuonKey: str = None,
        OutputMuonCellsKey: str = None,
        OutputMuonClusterKey: str = None,

        # ==================================================
        # CONFIGURAÇÕES GERAIS
        # ==================================================
        OutputLevel: int = 0,
        NtupleName: str = "CollectionTree",

        # ==================================================
        # FLAGS DE EXECUÇÃO
        # ==================================================
        EnableCaloOutput: bool = True,
        EnableMuonOutput: bool = True,
        DumpCells: bool = flags.DumpCells,
        DumpMuonCells: bool = False,
        DumpMuonClusters: bool = False,
    ):

        # Inicialização do componente C++
        Cpp.__init__(
            self,
            ROOT.RootStreamAODMaker(name)
        )

        # ==================================================
        # 1. ENTRADAS COMUNS
        # ==================================================

        self.setProperty("InputEventKey", InputEventKey)
        self.setProperty("InputTruthKey", InputTruthKey)
        self.setProperty("InputSeedsKey", InputSeedsKey)

        # ==================================================
        # 2. ENTRADAS DO CALORÍMETRO
        # ==================================================

        if InputCellsKey:
            self.setProperty("InputCellsKey", InputCellsKey)

        if InputTruthCellsKey:
            self.setProperty("InputTruthCellsKey", InputTruthCellsKey)

        if InputClusterKey:
            self.setProperty("InputClusterKey", InputClusterKey)

        if InputTruthClusterKey:
            self.setProperty("InputTruthClusterKey", InputTruthClusterKey)

        if InputRingerKeys:
            self.setProperty("InputRingerKeys", InputRingerKeys)

        if InputTruthRingerKeys:
            self.setProperty("InputTruthRingerKeys", InputTruthRingerKeys)

        if InputElectronKey:
            self.setProperty("InputElectronKey", InputElectronKey)

        if InputTruthElectronKey:
            self.setProperty("InputTruthElectronKey", InputTruthElectronKey)

        if InputRingerL0Key:
            self.setProperty("InputRingerL0Key", InputRingerL0Key)

        # ==================================================
        # 3. ENTRADAS DA CÂMARA DE MÚONS
        # ==================================================

        if InputMuonKey:
            self.setProperty("InputMuonKey", InputMuonKey)

        if InputMuonCellsKey:
            self.setProperty("InputMuonCellsKey", InputMuonCellsKey)

        if InputMuonClusterKey:
            self.setProperty("InputMuonClusterKey", InputMuonClusterKey)

        # ==================================================
        # 4. SAÍDAS COMUNS
        # ==================================================

        self.setProperty(
            "OutputEventKey",
            OutputEventKey if OutputEventKey else InputEventKey
        )

        self.setProperty(
            "OutputTruthKey",
            OutputTruthKey if OutputTruthKey else InputTruthKey
        )

        self.setProperty(
            "OutputSeedsKey",
            OutputSeedsKey if OutputSeedsKey else InputSeedsKey
        )

        # ==================================================
        # 5. SAÍDAS DO CALORÍMETRO
        # ==================================================

        if OutputCellsKey or InputCellsKey:
            self.setProperty(
                "OutputCellsKey",
                OutputCellsKey if OutputCellsKey else InputCellsKey
            )

        if OutputTruthCellsKey or InputTruthCellsKey:
            self.setProperty(
                "OutputTruthCellsKey",
                OutputTruthCellsKey if OutputTruthCellsKey else InputTruthCellsKey
            )

        if OutputClusterKey or InputClusterKey:
            self.setProperty(
                "OutputClusterKey",
                OutputClusterKey if OutputClusterKey else InputClusterKey
            )

        if OutputTruthClusterKey or InputTruthClusterKey:
            self.setProperty(
                "OutputTruthClusterKey",
                OutputTruthClusterKey if OutputTruthClusterKey else InputTruthClusterKey
            )

        if OutputRingerKeys or InputRingerKeys:
            self.setProperty(
                "OutputRingerKeys",
                OutputRingerKeys if OutputRingerKeys else InputRingerKeys
            )

        if OutputTruthRingerKeys or InputTruthRingerKeys:
            self.setProperty(
                "OutputTruthRingerKeys",
                OutputTruthRingerKeys if OutputTruthRingerKeys else InputTruthRingerKeys
            )

        if OutputElectronKey or InputElectronKey:
            self.setProperty(
                "OutputElectronKey",
                OutputElectronKey if OutputElectronKey else InputElectronKey
            )

        if OutputTruthElectronKey or InputTruthElectronKey:
            self.setProperty(
                "OutputTruthElectronKey",
                OutputTruthElectronKey if OutputTruthElectronKey else InputTruthElectronKey
            )

        if OutputRingerL0Key or InputRingerL0Key:
            self.setProperty(
                "OutputRingerL0Key",
                OutputRingerL0Key if OutputRingerL0Key else InputRingerL0Key
            )

        # ==================================================
        # 6. SAÍDAS DA CÂMARA DE MÚONS
        # ==================================================

        if OutputMuonKey:
            self.setProperty("OutputMuonKey", OutputMuonKey)
        elif InputMuonKey:
            self.setProperty("OutputMuonKey", InputMuonKey)

        if OutputMuonCellsKey:
            self.setProperty("OutputMuonCellsKey", OutputMuonCellsKey)
        elif InputMuonCellsKey:
            self.setProperty("OutputMuonCellsKey", InputMuonCellsKey)

        if OutputMuonClusterKey:
            self.setProperty("OutputMuonClusterKey", OutputMuonClusterKey)
        elif InputMuonClusterKey:
            self.setProperty("OutputMuonClusterKey", InputMuonClusterKey)

        # ==================================================
        # 7. CONFIGURAÇÕES GERAIS
        # ==================================================

        self.setProperty("OutputLevel", OutputLevel)
        self.setProperty("NtupleName", NtupleName)

        # ==================================================
        # 8. FLAGS DE EXECUÇÃO
        # ==================================================

        self.setProperty("EnableCaloOutput", EnableCaloOutput)
        self.setProperty("EnableMuonOutput", EnableMuonOutput)

        self.setProperty("DumpCells", DumpCells)
        self.setProperty("DumpMuonCells", DumpMuonCells)
        self.setProperty("DumpMuonClusters", DumpMuonClusters)

