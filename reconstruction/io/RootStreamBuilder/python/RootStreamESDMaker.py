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
                InputMuonCellKeys        : list = None,
                # --- Outputs (nomes oficiais preservados) ---
                OutputEventKey           : str = None,
                OutputTruthKey           : str = None,
                OutputSeedsKey           : str = None,
                OutputCellsKey           : str = None,
                OutputCellsTruthKey      : str = None,
                # --- Aliases da sua versao (opcionais) ---
                OutputCaloCellsKey       : str = None,
                OutputTruthCaloCellsKey  : str = None,   # <-- A CHAVE QUE FALTAVA
                # --- Aditivo: muon output ---
                OutputMuonCellsKey       : str = "MuonCells",
                # --- Oficial ---
                OutputLevel              : int = 1,
                NtupleName               : str = "CollectionTree",
                EtaWindow                : float = flags.EtaWindow,
                PhiWindow                : float = flags.PhiWindow,
                OnlyRoI                  : bool = flags.OnlyRoI,
                # --- Aditivo: flags de output (default preservam o comportamento oficial) ---
                EnableCaloOutput         : bool = True,
                EnableMuonOutput         : bool = True,
              ):

    Cpp.__init__(self, ROOT.RootStreamESDMaker(name))

    # Resolve aliases: alias tem precedencia se passado
    _in_cells        = InputCaloCellsKey      if InputCaloCellsKey      is not None else InputCellsKey
    _in_cells_truth  = InputTruthCaloCellsKey if InputTruthCaloCellsKey is not None else InputCellsTruthKey
    _out_cells       = OutputCaloCellsKey     if OutputCaloCellsKey     is not None else OutputCellsKey
    _out_cells_truth = OutputTruthCaloCellsKey if OutputTruthCaloCellsKey is not None else OutputCellsTruthKey

    # --- Inputs obrigatorios ---
    self.setProperty( "InputEventKey", InputEventKey )
    self.setProperty( "InputTruthKey", InputTruthKey )
    self.setProperty( "InputSeedsKey", InputSeedsKey )

    # --- Inputs de calorimetro: so seta se o usuario passou algo ---
    if _in_cells       is not None: self.setProperty( "InputCellsKey"      , _in_cells       )
    if _in_cells_truth is not None: self.setProperty( "InputCellsTruthKey" , _in_cells_truth )

    # --- Aliases de input (só se passados) ---
    if InputCaloCellsKey      is not None: self.setProperty( "InputCaloCellsKey"     , InputCaloCellsKey      )
    if InputTruthCaloCellsKey is not None: self.setProperty( "InputTruthCaloCellsKey", InputTruthCaloCellsKey )

    # --- Muon input (aditivo) ---
    if InputMuonCellsKey      is not None: self.setProperty( "InputMuonCellsKey"     , InputMuonCellsKey      )
    if InputTruthMuonCellsKey is not None: self.setProperty( "InputTruthMuonCellsKey", InputTruthMuonCellsKey )

    if InputMuonCellKeys:
      vec = ROOT.std.vector('std::string')()
      for key in InputMuonCellKeys:
        vec.push_back(key)
      self.setProperty( "InputMuonCellKeys", vec )

    # --- Outputs: fallback para o input equivalente quando nao especificado ---
    out_event = OutputEventKey if OutputEventKey is not None else InputEventKey
    out_truth = OutputTruthKey if OutputTruthKey is not None else InputTruthKey
    out_seeds = OutputSeedsKey if OutputSeedsKey is not None else InputSeedsKey
    self.setProperty( "OutputEventKey", out_event )
    self.setProperty( "OutputTruthKey", out_truth )
    self.setProperty( "OutputSeedsKey", out_seeds )

    if _out_cells       is not None: self.setProperty( "OutputCellsKey"      , _out_cells       )
    if _out_cells_truth is not None: self.setProperty( "OutputCellsTruthKey" , _out_cells_truth )

    # --- Aliases de output (so se passados) ---
    if OutputCaloCellsKey      is not None: self.setProperty( "OutputCaloCellsKey"     , OutputCaloCellsKey      )
    if OutputTruthCaloCellsKey is not None: self.setProperty( "OutputTruthCaloCellsKey", OutputTruthCaloCellsKey )  # <-- resolve o TypeError

    # --- Muon output (aditivo) ---
    if OutputMuonCellsKey is not None:
      self.setProperty( "OutputMuonCellsKey", OutputMuonCellsKey )

    self.setProperty( "OutputLevel", OutputLevel )
    self.setProperty( "NtupleName" , NtupleName  )
    self.setProperty( "EtaWindow"  , EtaWindow   )
    self.setProperty( "PhiWindow"  , PhiWindow   )
    self.setProperty( "OnlyRoI"    , OnlyRoI     )

    self.setProperty( "EnableCaloOutput", EnableCaloOutput )
    self.setProperty( "EnableMuonOutput", EnableMuonOutput )
