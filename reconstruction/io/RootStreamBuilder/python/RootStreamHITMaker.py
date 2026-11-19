__all__ = ["RootStreamHITMaker"]

import ROOT

from typing import List
from GaugiKernel import Cpp
from GaugiKernel.macros import *
from RootStreamBuilder import RootStreamHITFlags as flags
from RootStreamBuilder import RootStreamESDFlags as flags_ESD

class RootStreamHITMaker( Cpp ):
  def __init__( self, name,
                InputEventKey    : str,
                InputTruthKey    : str,
                InputSeedsKey    : str,
                # --- Oficial: hits do calorimetro ---
                InputHitsKey     : str = None,
                # --- Alias da sua versao (opcional; se passado, tem precedencia) ---
                InputCaloHitsKey : str = None,
                # --- Aditivo: hits de muon ---
                InputMuonHitsKey : str = None,            
                OutputEventKey   : str = None,
                OutputTruthKey   : str = None,
                # --- Oficial ---
                OutputHitsKey    : str = None,
                # --- Alias da sua versao (opcional) ---
                OutputCaloHitsKey: str = None,
                # --- Aditivo: hits de muon ---
                OutputMuonHitsKey: str = None,
                OutputSeedsKey   : str = None,
                OutputLevel      : int = 0, 
                NtupleName       : str = "CollectionTree",
                OnlyRoI          : bool = flags.OnlyRoI,
                EtaWindow        : float = flags.EtaWindow,
                PhiWindow        : float = flags.PhiWindow,
                # --- Aditivo: janela de muon ---
                MuonEtaWindow    : float = 0.6,
                MuonPhiWindow    : float = 0.6,
                EnableMuonOutput : bool = True,
                # --- Oficial ---
                KeepCells        : List[int] = None,
              ): 
    
    Cpp.__init__(self, ROOT.Gaugi.RootStreamHITMaker(name))

    # Resolve aliases (sua versao tem precedencia se passada)
    _in_hits  = InputCaloHitsKey  if InputCaloHitsKey  is not None else InputHitsKey
    _out_hits = OutputCaloHitsKey if OutputCaloHitsKey is not None else OutputHitsKey
    
    self.setProperty( "InputEventKey"   , InputEventKey   )
    self.setProperty( "InputTruthKey"   , InputTruthKey   )
    # Oficial: InputHitsKey (com alias InputCaloHitsKey)
    self.setProperty( "InputHitsKey"    , _in_hits if _in_hits is not None else "" )
    # Aditivo: InputMuonHitsKey
    if InputMuonHitsKey:
      self.setProperty( "InputMuonHitsKey", InputMuonHitsKey )
    self.setProperty( "InputSeedsKey"   , InputSeedsKey   )
    self.setProperty( "OutputEventKey"  , OutputEventKey if OutputEventKey else InputEventKey  )
    self.setProperty( "OutputTruthKey"  , OutputTruthKey if OutputTruthKey else InputTruthKey  )
    # Oficial: OutputHitsKey (com alias OutputCaloHitsKey)
    self.setProperty( "OutputHitsKey"   , _out_hits if _out_hits is not None else (_in_hits if _in_hits else "") )
    # Aditivo: OutputMuonHitsKey
    if OutputMuonHitsKey:
      self.setProperty( "OutputMuonHitsKey", OutputMuonHitsKey )
    self.setProperty( "OutputSeedsKey"  , OutputSeedsKey if OutputSeedsKey else InputSeedsKey       )
    self.setProperty( "OutputLevel"     , OutputLevel     ) 
    self.setProperty( "NtupleName"      , NtupleName      )
    self.setProperty( "OnlyRoI"         , OnlyRoI         )
    self.setProperty( "EtaWindow"       , EtaWindow       )
    self.setProperty( "PhiWindow"       , PhiWindow       )
    # Aditivo: janela e flag de muon
    self.setProperty( "MuonEtaWindow"   , MuonEtaWindow   )
    self.setProperty( "MuonPhiWindow"   , MuonPhiWindow   )
    self.setProperty( "EnableMuonOutput", EnableMuonOutput )
    # Oficial: KeepCells
    if KeepCells: 
      self.setProperty( "KeepCells"     , KeepCells       )
