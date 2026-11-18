__all__ = ["RootStreamAODMaker"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
from RootStreamBuilder import RootStreamAODFlags as flags
import ROOT

class RootStreamAODMaker( Cpp ):

  def __init__( self, name,
                InputEventKey        : str,
                InputTruthKey        : str,
                InputSeedsKey        : str,
                InputCellsKey        : str = None,
                InputClusterKey      : str = None,
                InputRingerKeys      : list = None,
                InputElectronKey     : str = None,
                # --- Aditivo: chaves de muon (opcionais) ---
                InputMuonKey         : str = None,
                InputMuonCellsKey    : str = None,
                InputMuonClusterKey  : str = None,
                OutputEventKey       : str = None,
                OutputTruthKey       : str = None,
                OutputSeedsKey       : str = None,
                OutputCellsKey       : str = None,
                OutputClusterKey     : str = None,
                OutputRingerKeys     : list = None,
                OutputElectronKey    : str = None,
                # --- Aditivo: chaves de muon (opcionais) ---
                OutputMuonKey        : str = None,
                OutputMuonCellsKey   : str = None,
                OutputMuonClusterKey : str = None,
                OutputLevel          : int = 0, 
                NtupleName           : str = "CollectionTree",
                # --- Aditivo: flags de output (default preservam o comportamento oficial) ---
                EnableCaloOutput     : bool = True,
                EnableMuonOutput     : bool = True,
                DumpCells            : bool = flags.DumpCells,
                DumpMuonCells        : bool = False,
                DumpMuonClusters     : bool = False,
              ): 
    
    Cpp.__init__(self, ROOT.RootStreamAODMaker(name))
    
    # Calorimeter inputs (apenas se fornecidos) -- IGUAL AO OFICIAL
    self.setProperty( "InputEventKey"    , InputEventKey     )
    self.setProperty( "InputTruthKey"    , InputTruthKey     )
    self.setProperty( "InputSeedsKey"    , InputSeedsKey     )
    
    if InputCellsKey:
        self.setProperty( "InputCellsKey"    , InputCellsKey     )
    if InputClusterKey:
        self.setProperty( "InputClusterKey"  , InputClusterKey   )
    if InputRingerKeys:
        self.setProperty( "InputRingerKeys"  , InputRingerKeys   )
    if InputElectronKey:
        self.setProperty( "InputElectronKey" , InputElectronKey  )
    
    # --- Aditivo: muon inputs (opcionais) ---
    if InputMuonKey:
        self.setProperty( "InputMuonKey", InputMuonKey )
    if InputMuonCellsKey:
        self.setProperty( "InputMuonCellsKey", InputMuonCellsKey )
    if InputMuonClusterKey:
        self.setProperty( "InputMuonClusterKey", InputMuonClusterKey )

    # Calorimeter outputs -- IGUAL AO OFICIAL
    self.setProperty( "OutputEventKey"    , OutputEventKey if OutputEventKey else InputEventKey       )
    self.setProperty( "OutputTruthKey"    , OutputTruthKey if OutputTruthKey else InputTruthKey       )
    self.setProperty( "OutputSeedsKey"    , OutputSeedsKey if OutputSeedsKey else InputSeedsKey       )
    
    if OutputCellsKey or InputCellsKey:
        self.setProperty( "OutputCellsKey"    , OutputCellsKey if OutputCellsKey else InputCellsKey       )
    if OutputClusterKey or InputClusterKey:
        self.setProperty( "OutputClusterKey"  , OutputClusterKey if OutputClusterKey else InputClusterKey )
    if OutputRingerKeys or InputRingerKeys:
        self.setProperty( "OutputRingerKeys"  , OutputRingerKeys if OutputRingerKeys else InputRingerKeys )
    if OutputElectronKey or InputElectronKey:
        self.setProperty( "OutputElectronKey" , OutputElectronKey if OutputElectronKey else InputElectronKey)
    
    # --- Aditivo: muon outputs (opcionais) ---
    if OutputMuonKey:
        self.setProperty( "OutputMuonKey", OutputMuonKey )
    elif InputMuonKey:
        self.setProperty( "OutputMuonKey", InputMuonKey )
        
    if OutputMuonCellsKey:
        self.setProperty( "OutputMuonCellsKey", OutputMuonCellsKey )
    elif InputMuonCellsKey:
        self.setProperty( "OutputMuonCellsKey", InputMuonCellsKey )
        
    if OutputMuonClusterKey:
        self.setProperty( "OutputMuonClusterKey", OutputMuonClusterKey )
    elif InputMuonClusterKey:
        self.setProperty( "OutputMuonClusterKey", InputMuonClusterKey )
    
    # General settings -- IGUAL AO OFICIAL + flags aditivas
    self.setProperty( "OutputLevel"       , OutputLevel       ) 
    self.setProperty( "NtupleName"        , NtupleName        )
    # --- Aditivo: flags de output (default preservam o comportamento oficial) ---
    self.setProperty( "EnableCaloOutput"  , EnableCaloOutput  )
    self.setProperty( "EnableMuonOutput"  , EnableMuonOutput  )
    self.setProperty( "DumpCells"         , DumpCells         )
    self.setProperty( "DumpMuonCells"     , DumpMuonCells     )     
    self.setProperty( "DumpMuonClusters"  , DumpMuonClusters  )
