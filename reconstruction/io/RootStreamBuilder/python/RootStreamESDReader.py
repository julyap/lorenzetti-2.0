__all__ = ["RootStreamESDReader"]

from GaugiKernel import Cpp
from GaugiKernel.macros import *
import ROOT

class RootStreamESDReader( Cpp ):

  def __init__( self, name,
                InputFile           : str,
                OutputEventKey      : str,
                OutputTruthKey      : str,
                OutputSeedsKey      : str,
                # --- Calorimeter (nomes oficiais preservados) ---
                OutputCellsKey      : str = None,
                OutputCellsTruthKey : str = None,
                # --- Alias da sua versao (opcional; se passado, tem precedencia) ---
                OutputCaloCellsKey  : str = None,
                # --- Aditivo: muon ---
                OutputMuonCellsKey  : str = "MuonCells",
                # --- Oficial ---
                OutputLevel         : int = 0,
                NtupleName          : str = "CollectionTree",
                # --- Aditivo: flags (default preservam o comportamento oficial) ---
                EnableCaloOutput    : bool = True,
                EnableMuonOutput    : bool = True,
                MuonCellKeys        : list = [],
              ): 

    Cpp.__init__(self, ROOT.RootStreamESDReader(name))

    # Resolve alias (sua versao tem precedencia se passada)
    _out_cells = OutputCaloCellsKey if OutputCaloCellsKey is not None else OutputCellsKey

    self.setProperty( "OutputEventKey"      , OutputEventKey      )
    self.setProperty( "OutputTruthKey"      , OutputTruthKey      )
    self.setProperty( "OutputSeedsKey"      , OutputSeedsKey      )
    # Calorimeter (nomes oficiais preservados)
    self.setProperty( "OutputCellsKey"      , _out_cells if _out_cells is not None else "" )
    self.setProperty( "OutputCellsTruthKey" , OutputCellsTruthKey if OutputCellsTruthKey is not None else "" )
    # Muon (aditivo)
    self.setProperty( "OutputMuonCellsKey"  , OutputMuonCellsKey  )
    self.setProperty( "OutputLevel"         , OutputLevel         ) 
    self.setProperty( "NtupleName"          , NtupleName          )
    self.setProperty( "InputFile"           , InputFile           )
    # Flags (aditivo)
    self.setProperty( "EnableCaloOutput"    , EnableCaloOutput    )
    self.setProperty( "EnableMuonOutput"    , EnableMuonOutput    )

    # Configurar multiplas chaves de muon se fornecidas
    if MuonCellKeys:
        self.setProperty( "MuonCellKeys", MuonCellKeys )
        self.setProperty( "EnableMuonOutput", True )

    # Armazenar para uso posterior
    self.InputFile = InputFile
    self.NtupleName = NtupleName

    # Verificar arquivo e obter numero de entradas
    f = ROOT.TFile.Open( InputFile, "read" )
    if not f or f.IsZombie():
        raise RuntimeError( f"Could not open file: {InputFile}" )

    t = f.Get( NtupleName )
    if not t:
        raise RuntimeError( f"Could not find tree '{NtupleName}' in file: {InputFile}" )

    self.__entries = t.GetEntries()
    f.Close()


  def GetEntries(self):
    return self.__entries


  def merge(self, acc):
    acc.SetReader(self)
