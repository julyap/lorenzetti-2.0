__all__ = ["RootStreamHITReader"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
import ROOT


class RootStreamHITReader( Cpp ):

  def __init__( self, name,
                OutputEventKey    : str,
                OutputTruthKey    : str,
                # --- Oficial: hits do calorimetro ---
                OutputHitsKey     : str = None,
                # --- Alias da sua versao (opcional; se passado, tem precedencia) ---
                OutputCaloHitsKey : str = None,
                # --- Aditivo: hits de muon ---
                OutputMuonHitsKey : str = None,
                OutputSeedsKey    : str = None,
                InputFile         : str = None,
                OutputLevel       : int = 0, 
                NtupleName        : str = "CollectionTree",
                # --- Aditivo: flag de muon ---
                EnableMuonInput   : bool = False,
              ): 
    
    Cpp.__init__(self, ROOT.RootStreamHITReader(name))

    # Resolve alias (sua versao tem precedencia se passada)
    _out_hits = OutputCaloHitsKey if OutputCaloHitsKey is not None else OutputHitsKey

    self.setProperty( "OutputEventKey"  , OutputEventKey  )
    self.setProperty( "OutputTruthKey"  , OutputTruthKey  )
    # Oficial: OutputHitsKey (com alias OutputCaloHitsKey)
    self.setProperty( "OutputHitsKey"   , _out_hits if _out_hits is not None else "" )
    # Aditivo: OutputMuonHitsKey
    if OutputMuonHitsKey:
      self.setProperty( "OutputMuonHitsKey", OutputMuonHitsKey )
    self.setProperty( "OutputSeedsKey"  , OutputSeedsKey if OutputSeedsKey is not None else "" )
    self.setProperty( "OutputLevel"     , OutputLevel     ) 
    self.setProperty( "NtupleName"      , NtupleName      )
    self.setProperty( "InputFile"       , InputFile       )
    # Aditivo: flag de muon
    self.setProperty( "EnableMuonInput" , EnableMuonInput )

    f = ROOT.TFile( self.InputFile,"read")
    t = f.Get( self.NtupleName)
    self.__entries = t.GetEntries()


  def GetEntries(self):
    return self.__entries


  def merge(self, acc):
    acc.SetReader(self)
