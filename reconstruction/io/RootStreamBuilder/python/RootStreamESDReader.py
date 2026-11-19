
__all__ = ["RootStreamESDReader"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *

import ROOT


class RootStreamESDReader(Cpp):

    def __init__(
        self,
        name,
        InputFile: str,

        OutputEventKey: str,
        OutputTruthKey: str,
        OutputSeedsKey: str,

        InputEventBranchKey: str = "EventInfo",

        OutputCellsKey: str = None,
        OutputCellsTruthKey: str = None,

        OutputCaloCellsKey: str = None,
        OutputCaloCellsTruthKey: str = None,
        OutputTruthCaloCellsKey: str = None,

        OutputMuonCellsKey: str = "MuonCells",
        MuonCellKeys: list = None,

        OutputLevel: int = 0,
        NtupleName: str = "CollectionTree",

        EnableCaloOutput: bool = True,
        EnableMuonOutput: bool = True,
    ):

        Cpp.__init__(
            self,
            ROOT.RootStreamESDReader(name)
        )


        # --------------------------------------------------
        # Resolver aliases do calorímetro
        # --------------------------------------------------

        cells_key = (
            OutputCaloCellsKey
            if OutputCaloCellsKey is not None
            else (
                OutputCellsKey
                if OutputCellsKey is not None
                else "Hits"
            )
        )

        cells_truth_key = (
            OutputCaloCellsTruthKey
            if OutputCaloCellsTruthKey is not None
            else (
                OutputTruthCaloCellsKey
                if OutputTruthCaloCellsKey is not None
                else (
                    OutputCellsTruthKey
                    if OutputCellsTruthKey is not None
                    else "CellsTruth"
                )
            )
        )


        # --------------------------------------------------
        # Propriedades oficiais
        # --------------------------------------------------

        self.setProperty("InputFile", InputFile)

        self.setProperty("OutputEventKey", OutputEventKey)

        # NOVO: branch de entrada independente da chave de saída
        self.setProperty(
            "InputEventBranchKey",
            InputEventBranchKey
        )

        self.setProperty("OutputTruthKey", OutputTruthKey)

        self.setProperty("OutputSeedsKey", OutputSeedsKey)


        # --------------------------------------------------
        # Calorímetro
        # --------------------------------------------------

        self.setProperty("OutputCellsKey", cells_key)

        self.setProperty("OutputCaloCellsKey", cells_key)

        self.setProperty(
            "OutputCellsTruthKey",
            cells_truth_key
        )

        self.setProperty(
            "OutputTruthCaloCellsKey",
            cells_truth_key
        )


        # --------------------------------------------------
        # Muon
        # --------------------------------------------------

        self.setProperty(
            "OutputMuonCellsKey",
            OutputMuonCellsKey
        )

        if MuonCellKeys:

            self.setProperty(
                "OutputMuonCellKeys",
                MuonCellKeys
            )


        # --------------------------------------------------
        # Configurações
        # --------------------------------------------------

        self.setProperty(
            "EnableCaloOutput",
            EnableCaloOutput
        )

        self.setProperty(
            "EnableMuonOutput",
            EnableMuonOutput
        )

        self.setProperty(
            "OutputLevel",
            OutputLevel
        )

        self.setProperty(
            "NtupleName",
            NtupleName
        )


        # --------------------------------------------------
        # Verificar arquivo ROOT
        # --------------------------------------------------

        self.InputFile = InputFile

        self.NtupleName = NtupleName

        f = ROOT.TFile.Open(
            InputFile,
            "read"
        )

        if not f or f.IsZombie():

            raise RuntimeError(
                f"Could not open file: {InputFile}"
            )

        try:

            t = f.Get(NtupleName)

            if not t:

                raise RuntimeError(
                    f"Could not find tree {NtupleName} "
                    f"in file {InputFile}"
                )

            self.__entries = t.GetEntries()

        finally:

            f.Close()


    # ------------------------------------------------------
    # Número de eventos
    # ------------------------------------------------------

    def GetEntries(self):

        return self.__entries


    # ------------------------------------------------------
    # Merge
    # ------------------------------------------------------

    def merge(self, acc):

        acc.SetReader(self)

