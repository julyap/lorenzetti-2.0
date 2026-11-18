__all__ = ["RootStreamESDMaker"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
from RootStreamBuilder import RootStreamESDFlags as flags
import ROOT

class RootStreamESDMaker( Cpp ):

  def __init__( self, name,
                InputEventKey            : str,
                InputTruthKey            : str,
                InputSeedsKey            : str,
                # --- Calorimeter (nomes oficiais preservados) ---
                InputCellsKey            : str = None,
                InputCellsTruthKey       : str = None,
                # --- Aliases da sua versao (opcionais; se passados, tem precedencia) ---
                InputCaloCellsKey        : str = None,
                InputTruthCaloCellsKey   : str = None,
                # --- Aditivo: muon ---
                InputMuonCellsKey        : str = "",
                InputTruthMuonCellsKey   : str = "",
                InputMuonCellKeys        : list = [],
                # --- Outputs (nomes oficiais preservados) ---
                OutputEventKey           : str = None,
                OutputTruthKey           : str = None,
                OutputSeedsKey           : str = None,
                OutputCellsKey           : str = None,
                OutputCellsTruthKey      : str = None,
                # --- Aliases da sua versao (opcionais) ---
                OutputCaloCellsKey       : str = None,
                # --- Aditivo: muon output ---
                OutputMuonCellsKey       : str = "MuonCells",
                # --- Oficial ---
                OutputLevel              : int = 0,
                NtupleName               : str = "CollectionTree",
                EtaWindow                : float = flags.EtaWindow,
                PhiWindow                : float = flags.PhiWindow,
                OnlyRoI                  : bool = flags.OnlyRoI,
                # --- Aditivo: flags de output (default preservam o comportamento oficial) ---
                EnableCaloOutput         : bool = True,
                EnableMuonOutput         : bool = True,
              ): 
    """
    @brief Algorithm to stream ESD Data to a ROOT file.

    ESD contains detailed information including all Calorimeter Cells,
    which makes it larger than AOD but necessary for full reconstruction study.
    This algo serializes the Event Context into a TTree.

    Properties (oficiais):
    - InputEventKey: Key for the EventInfo container.
    - InputTruthKey: Key for the TruthParticle container.
    - InputCellsKey: Key for the CaloCell container.
    - InputSeedsKey: Key for the Seed container.
    - InputCellsTruthKey: Key for the CaloCellTruth container.
    - OutputEventKey: Key for the EventInfo container.
    - OutputTruthKey: Key for the TruthParticle container.
    - OutputCellsKey: Key for the CaloCell container.
    - OutputSeedsKey: Key for the Seed container.
    - OutputCellsTruthKey: Key for the CaloCellTruth container.
    - OutputLevel: Level of the output messages.
    - NtupleName: Name of the TTree.
    - EtaWindow: Eta window for the RoI.
    - PhiWindow: Phi window for the RoI.
    - OnlyRoI: If True, only saves the cells inside the RoI around the truth particles; if False, saves all cells.

    Properties (aditivas, muon):
    - InputMuonCellsKey: Key for the MuonCell container.
    - InputTruthMuonCellsKey: Key for the TruthMuonCell container.
    - InputMuonCellKeys: list of keys for multiple muon containers.
    - OutputMuonCellsKey: Key for the output MuonCell container.
    - EnableCaloOutput: If True, writes calorimeter cells.
    - EnableMuonOutput: If True, writes muon cells.
    """
    Cpp.__init__(self, ROOT.RootStreamESDMaker(name))

    # Resolve aliases (sua versao tem precedencia se passada)
    _in_cells       = InputCaloCellsKey      if InputCaloCellsKey      is not None else InputCellsKey
    _in_cells_truth = InputTruthCaloCellsKey if InputTruthCaloCellsKey is not None else InputCellsTruthKey
    _out_cells      = OutputCaloCellsKey     if OutputCaloCellsKey     is not None else OutputCellsKey

    self.setProperty( "InputEventKey"           , InputEventKey                                                       )
    self.setProperty( "InputTruthKey"           , InputTruthKey                                                       )
    self.setProperty( "InputSeedsKey"           , InputSeedsKey                                                       )
    # Calorimeter (nomes oficiais preservados)
    self.setProperty( "InputCellsKey"           , _in_cells       if _in_cells       is not None else "" )
    self.setProperty( "InputCellsTruthKey"      , _in_cells_truth if _in_cells_truth is not None else "" )
    # Muon (aditivo)
    self.setProperty( "InputMuonCellsKey"       , InputMuonCellsKey       )
    self.setProperty( "InputTruthMuonCellsKey"  , InputTruthMuonCellsKey  )

    # Converter lista Python para vector<string>
    if InputMuonCellKeys:
        vec = ROOT.std.vector('std::string')()
        for key in InputMuonCellKeys:
            vec.push_back(key)
        self.setProperty( "InputMuonCellKeys", vec )

    self.setProperty( "OutputEventKey"          , OutputEventKey if OutputEventKey else InputEventKey )
    self.setProperty( "OutputTruthKey"          , OutputTruthKey if OutputTruthKey else InputTruthKey )
    self.setProperty( "OutputSeedsKey"          , OutputSeedsKey if OutputSeedsKey else InputSeedsKey )
    # Calorimeter (nomes oficiais preservados)
    self.setProperty( "OutputCellsKey"          , _out_cells if _out_cells is not None else "" )
    self.setProperty( "OutputCellsTruthKey"     , OutputCellsTruthKey if OutputCellsTruthKey else (_in_cells_truth if _in_cells_truth else "") )
    # Muon (aditivo)
    self.setProperty( "OutputMuonCellsKey"      , OutputMuonCellsKey      )
    self.setProperty( "OutputLevel"             , OutputLevel             ) 
    self.setProperty( "NtupleName"              , NtupleName              )
    self.setProperty( "EtaWindow"               , EtaWindow               )
    self.setProperty( "PhiWindow"               , PhiWindow               )
    self.setProperty( "OnlyRoI"                 , OnlyRoI                 )
    # Flags (aditivo)
    self.setProperty( "EnableCaloOutput"        , EnableCaloOutput        ) 
    self.setProperty( "EnableMuonOutput"        , EnableMuonOutput        )
