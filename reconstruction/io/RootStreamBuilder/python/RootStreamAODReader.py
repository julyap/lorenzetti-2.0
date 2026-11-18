__all__ = ["RootStreamAODReader"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
import ROOT

class RootStreamAODReader( Cpp ):

  def __init__( self, name,
                InputFile              : str,
                OutputEventKey         : str,
                OutputTruthKey         : str,
                OutputSeedsKey         : str,
                OutputClusterKey       : str = None,
                OutputRingerKeys       : list = None,
                OutputElectronKey      : str = None,
                # --- Aditivo: chaves de muon (opcionais) ---
                OutputMuonKey          : str = None,
                OutputMuonClusterKey   : str = None,
                OutputTruthClusterKey  : str = None,
                OutputTruthRingerKeys  : list = None,
                OutputTruthElectronKey : str = None,
                OutputLevel            : int = 0, 
                NtupleName             : str = "CollectionTree",
                # --- Aditivo: flags (default preservam o comportamento oficial) ---
                EnableCaloOutput       : bool = True,
                EnableMuonOutput       : bool = True,
              ): 
    
    Cpp.__init__(self, ROOT.RootStreamAODReader(name))
    
    # Configuracoes padrao (sempre presentes) -- IGUAL AO OFICIAL
    self.setProperty( "OutputEventKey"   , OutputEventKey     )
    self.setProperty( "OutputTruthKey"   , OutputTruthKey     )
    self.setProperty( "OutputSeedsKey"   , OutputSeedsKey     )
    self.setProperty( "OutputLevel"      , OutputLevel        ) 
    self.setProperty( "NtupleName"       , NtupleName         )
    self.setProperty( "InputFile"        , InputFile          )
    
    # Calorimeter keys (opcionais) -- IGUAL AO OFICIAL
    if OutputClusterKey:
      self.setProperty( "OutputClusterKey" , OutputClusterKey   )
    if OutputRingerKeys:
      self.setProperty( "OutputRingerKeys" , OutputRingerKeys   )
    if OutputElectronKey:
      self.setProperty( "OutputElectronKey", OutputElectronKey  )
    if OutputTruthClusterKey:
      self.setProperty( "OutputTruthClusterKey", OutputTruthClusterKey )
    if OutputTruthRingerKeys:
      self.setProperty( "OutputTruthRingerKeys", OutputTruthRingerKeys )
    if OutputTruthElectronKey:
      self.setProperty( "OutputTruthElectronKey", OutputTruthElectronKey )
    
    # --- Aditivo: muon keys (opcionais) ---
    if OutputMuonKey:
      self.setProperty( "OutputMuonKey"       , OutputMuonKey      )
    if OutputMuonClusterKey:
      self.setProperty( "OutputMuonClusterKey", OutputMuonClusterKey)
    
    # --- Aditivo: flags (default preservam o comportamento oficial) ---
    self.setProperty( "EnableCaloOutput" , EnableCaloOutput   )
    self.setProperty( "EnableMuonOutput" , EnableMuonOutput   )
    
    # Contagem de eventos -- IGUAL AO OFICIAL
    f = ROOT.TFile(self.InputFile, "read")
    t = f.Get(self.NtupleName)
    self.__entries = t.GetEntries()

  def GetEntries(self):
    return self.__entries

  def merge(self, acc):
    acc.SetReader(self)
