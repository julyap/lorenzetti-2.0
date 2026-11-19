__all__ = ["RootStreamHITReader"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
import ROOT


class RootStreamHITReader( Cpp ):

  def __init__( self, name,
                OutputEventKey    : str,
                OutputTruthKey    : str,
                # --- alias mantido por compatibilidade ---
                OutputHitsKey     : str = None,
                # --- nome real no C++ ---
                OutputCaloHitsKey : str = None,
                OutputMuonHitsKey : str = None,
                OutputSeedsKey    : str = None,
                InputFile         : str = None,
                OutputLevel       : int = 0,
                NtupleName        : str = "CollectionTree",
                EnableMuonInput   : bool = False,
              ):

    Cpp.__init__(self, ROOT.Gaugi.RootStreamHITReader(name))

    # Resolve alias: OutputCaloHitsKey tem precedência; senão usa OutputHitsKey
    _out_calo = OutputCaloHitsKey if OutputCaloHitsKey is not None else OutputHitsKey

    self.setProperty( "OutputEventKey", OutputEventKey )
    self.setProperty( "OutputTruthKey", OutputTruthKey )

    # Só define se o usuário passou algo; caso contrário usa o default do C++ ("CaloHits")
    if _out_calo is not None:
      self.setProperty( "OutputCaloHitsKey", _out_calo )
    if OutputMuonHitsKey is not None:
      self.setProperty( "OutputMuonHitsKey", OutputMuonHitsKey )
    if OutputSeedsKey is not None:
      self.setProperty( "OutputSeedsKey", OutputSeedsKey )

    self.setProperty( "OutputLevel"      , OutputLevel     )
    self.setProperty( "NtupleName"       , NtupleName      )
    self.setProperty( "InputFile"        , InputFile       )
    self.setProperty( "EnableMuonInput"  , EnableMuonInput )

    f = ROOT.TFile( self.InputFile, "read" )
    t = f.Get( self.NtupleName )
    self.__entries = t.GetEntries()


  def GetEntries(self):
    return self.__entries


  def merge(self, acc):
    acc.SetReader(self)
