__all__ = [
           "DetectorConstruction_v1", 
           ]


import ROOT

from typing import List
from prettytable import PrettyTable
from pprint import pprint
from tqdm import tqdm

from GaugiKernel.constants import *
from GaugiKernel import Cpp
from GaugiKernel.macros import *

from geometry.v1.PhysicalVolume   import Plates
from geometry.v1.ECAL             import getLArBarrelCfg
from geometry.v1.TILE             import getTileBarrelCfg, getTileExtendedCfg
from geometry.v1.EMEC             import getLArEMECCfg
from geometry.v1.HEC              import getHECCfg
from geometry.v1.DeadMaterials    import getDMVolumesCfg, getCrackVolumesCfg
# --- Aditivo: camara de muons ---
from geometry.v1.MuonChamber      import getMuonBarrelCfg
from geometry.v1.MuonEndcap       import getMuonEndcapCfg
#from geometry.detectors.Tracking      import *


def flatten(nested_list : List) -> List:
  """Flatten a nested list."""
  result = []
  for item in nested_list:
    if isinstance(item, list):
      result.extend(flatten(item))
    else:
      result.append(item)
  return result


class DetectorConstruction_v1( Cpp ):

  def __init__( self, 
                name              : str, 
                UseMagneticField  : bool=False, 
                CutOnPhi          : bool=False,
              ):

    Cpp.__init__(self, ROOT.DetectorConstruction_v1(name) ) 
    
    self.setProperty( "UseMagneticField", UseMagneticField  )
    self.setProperty( "CutOnPhi"        , CutOnPhi          )

    self.samplings = []
    self.volumes = []
    # --- Aditivo: camara de muons ---
    self.muon_chambers = []
    # Center
    
    #volumes.extend( getPixelBarrelCfg()   )
    self.samplings.extend( getLArBarrelCfg()   )
    self.samplings.extend( getTileBarrelCfg()  )
    self.volumes.extend( getDMVolumesCfg()      )
    # Right side (A)
    self.samplings.extend( getTileExtendedCfg()    )
    self.samplings.extend( getLArEMECCfg()         ) 
    self.samplings.extend( getHECCfg()             )
    self.volumes.extend( getCrackVolumesCfg()      )
    # Left side (B)
    self.samplings.extend( getTileExtendedCfg(left_side = True) )
    self.samplings.extend( getLArEMECCfg(left_side=True)        ) 
    self.samplings.extend( getHECCfg(left_side=True)            )    
    self.volumes.extend( getCrackVolumesCfg(left_side=True)     )
    self.samplings = flatten(self.samplings)
    
    # --- Aditivo: camara de muons (barrel + endcap A + endcap C) ---
    self.muon_chambers.extend( getMuonBarrelCfg()                   )
    self.muon_chambers.extend( getMuonEndcapCfg()                   )  # Endcap A (+z)
    self.muon_chambers.extend( getMuonEndcapCfg(left_side=True)     )  # Endcap C (-z)
    
  
  def compile(self):
    # Create all volumes inside of the detector
    
    volumes = [pv for pv in self.volumes]
    volumes.extend( [samp.volume() for samp in self.samplings] )
    # Each volume once: samplings that share a volume (the two segments of each HEC layer, the segments of the EMEC samplings) would
    # place it more than once, one copy on top of the other
    unique = {}
    for pv in volumes: unique.setdefault( id(pv), pv )
    volumes = list( unique.values() )
          
    for pv in tqdm( volumes, desc="Compiling  volumes...", ncols=70):
      self._core.AddVolume( pv.Name, pv.Plates, pv.AbsorberMaterial, pv.GapMaterial, 
                             # layer
                             pv.NofLayers, 
                             pv.AbsorberThickness, 
                             pv.GapThickness,
                             # dimensions
                             pv.RMin, pv.RMax, pv.ZSize, 
                             pv.X, pv.Y, pv.Z,
                             # production cuts
                             pv.Cuts.ElectronCut, 
                             pv.Cuts.PositronCut, 
                             pv.Cuts.GammaCut, 
                             pv.Cuts.PhotonCut
                             )

    # --- Aditivo: camara de muons ---
    for chamber in tqdm( self.muon_chambers, desc="Compiling muon chambers...", ncols=70):
      pv = chamber.volume()
      self._core.AddMuonChamber(
          pv.Name,
          getattr(pv, 'ChamberType', 0),
          getattr(pv, 'RMin', 0),
          getattr(pv, 'RMax', 1.0),
          getattr(pv, 'ZSize', 1.0),
          getattr(pv, 'Z', 0),
          getattr(pv, 'ChamberStructure', 'G4_Al'),
          getattr(pv, 'GasMixture', 'G4_Ar'),
          getattr(pv, 'OperatingVoltage', 10000.0),
          getattr(pv, 'GasPressure', 1.0),
          getattr(pv, 'MuonCut', 1.0*cm) if getattr(pv, 'MuonCut', None) is not None else 1.0*cm
      )

  def summary(self):

      print('Display all calorimeter samplings...')
      t = PrettyTable(["Name", "Plates", "z",'Zmin','Zmax', "Rmin", 
                       "Rmax", "abso","gap", "deta", "dphi", "EtaMin", 
                       "EtaMax", "N_bins", "Container"])
      # Add all volumes that came from a sampling detector and has a sensitive parameter
      for samp in self.samplings:
        pv = samp.volume(); sv = samp.sensitive()
        t.add_row( [pv.Name,
                    Plates.tostring(pv.Plates),pv.ZSize,pv.ZMin,pv.ZMax,pv.RMin,pv.RMax,
                    pv.AbsorberMaterial,pv.GapMaterial,
                    round(sv.DeltaEta,4) ,
                    round(sv.DeltaPhi,4) ,
                    sv.EtaMin,sv.EtaMax, 
                    len(sv.EtaBins)*len(sv.PhiBins) if sv.DeltaEta else len(sv.ZBins)*len(sv.PhiBins)  ,
                    samp.CollectionKey
                  ])
      print(t)
      print('Display all non-sensitive volumes...')
      t = PrettyTable(["Name", "Plates", "z",'Zmin','Zmax', "Rmin", 
                       "Rmax", "abso","gap"])
      # Add ither volumes that not came from a sampling detector (extra volumes only)
      for pv in self.volumes:
        t.add_row([pv.Name, Plates.tostring(pv.Plates),pv.ZSize, pv.ZMin, pv.ZMax, 
                   pv.RMin, pv.RMax, pv.AbsorberMaterial, pv.GapMaterial]) 
      print(t)

      # --- Aditivo: camara de muons ---
      print('Display all muon chambers...')
      t = PrettyTable(["Name", "Type", "Rmin", "Rmax", "z", "ZSize",
                       "ChamberStructure", "GasMixture",
                       "OperatingVoltage", "GasPressure", "MuonCut", "Container"])
      for chamber in self.muon_chambers:
        pv = chamber.volume()
        ctype = getattr(pv, 'ChamberType', 0)
        t.add_row([
          pv.Name,
          ctype,
          pv.RMin, pv.RMax, pv.Z, pv.ZSize,
          getattr(pv, 'ChamberStructure', '-'),
          getattr(pv, 'GasMixture', '-'),
          getattr(pv, 'OperatingVoltage', 0.0),
          getattr(pv, 'GasPressure', 1.0),
          getattr(pv, 'MuonCut', 0.0),
          getattr(chamber, 'CollectionKey', '-'),
        ])
      print(t)

  def get_ui_commands(self) -> List[str]:

    commands = [
     "/vis/open OGL 600x600-0+0"
    ,"/vis/viewer/set/autoRefresh false"
    ,"/vis/verbose errors"
    ,"/vis/drawVolume"
    ,"/vis/viewer/set/viewpointVector 1 0 0"
    ,"/vis/viewer/set/lightsVector 1 0 0"
    ,"/vis/viewer/set/style wireframe"
    ,"/vis/viewer/set/auxiliaryEdge true"
    ,"/vis/viewer/set/lineSegmentsPerCircle 100"
    ,"/vis/scene/add/trajectories smooth"
    ,"/vis/modeling/trajectories/create/drawByCharge"
    ,"/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true"
    ,"/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 2"
    ,"/vis/scene/endOfEventAction accumulate"
    ,"/vis/geometry/set/visibility World 0 false"
    ,"#/vis/geometry/set/visibility World 0 true"
    ,"/vis/ogl/set/displayListLimit 10000000"
    ]
  
    def _add_volume_vis_commands(name, color, visualization) -> List[str]:
      vis_command = [
         f"/vis/geometry/set/colour {name} 0 {color}"
        ,f"/vis/geometry/set/colour {name}_Layer 0 {color}"
        ,f"/vis/geometry/set/colour {name}_Abso 0 {color}"
        ,f"/vis/geometry/set/colour {name}_Gap 0 {color}"
        ,f"/vis/geometry/set/visibility {name} 0 {visualization}"
        ,f"/vis/geometry/set/visibility {name}_Layer 0 {visualization}"
        ,f"/vis/geometry/set/visibility {name}_Abso 0 {visualization}"
        ,f"/vis/geometry/set/visibility {name}_Gap 0 {visualization}"
      ]
      return vis_command

    for samp in self.samplings:
      vol = samp.volume()
      commands.extend( _add_volume_vis_commands( vol.name(), vol.Color, 'true' if vol.Visualization else 'false') )
    for vol in self.volumes:
      commands.extend( _add_volume_vis_commands( vol.name(), vol.Color, 'true' if vol.Visualization else 'false') )

    # --- Aditivo: camara de muons ---
    # O C++ cria os volumes "<name>", "<name>_Structure" e "<name>_Gas". O comando de cor
    # do Geant4 espera valores numericos (r g b), mas os modulos de muons passam strings
    # ('steelblue', etc.). Aqui convertemos para RGB padrao, com fallback cinza.
    _color_map = {
        'steelblue':      '0.27 0.51 0.71',
        'royalblue':      '0.25 0.41 0.88',
        'darkblue':       '0.00 0.00 0.55',
        'lightblue':      '0.68 0.85 0.90',
        'skyblue':        '0.53 0.81 0.92',
        'lightsteelblue': '0.69 0.77 0.87',
        'cornflowerblue': '0.39 0.58 0.93',
        'gray':           '0.50 0.50 0.50',
        'grey':           '0.50 0.50 0.50',
    }
    for chamber in self.muon_chambers:
      pv = chamber.volume()
      name = pv.name() if hasattr(pv, 'name') else pv.Name
      raw = getattr(pv, 'Color', None)
      color = _color_map.get(raw, '0.50 0.50 0.50') if isinstance(raw, str) else (
              f"{raw} {raw} {raw}" if isinstance(raw, (int, float)) else '0.50 0.50 0.50')
      vis = 'true' if getattr(pv, 'Visualization', True) else 'false'
      commands.extend([
         f"/vis/geometry/set/colour {name} 0 {color}"
        ,f"/vis/geometry/set/colour {name}_Structure 0 {color}"
        ,f"/vis/geometry/set/colour {name}_Gas 0 {color}"
        ,f"/vis/geometry/set/visibility {name} 0 {vis}"
        ,f"/vis/geometry/set/visibility {name}_Structure 0 {vis}"
        ,f"/vis/geometry/set/visibility {name}_Gas 0 {vis}"
      ])

    commands.extend( [
             "/vis/viewer/set/autoRefresh true"
            ,"/vis/verbose warnings"
            ])

    return commands




if __name__ == "__main__":
    atlas = DetectorConstruction_v1("ATLAS")
    atlas.summary()
    #atlas.compile()
    #pprint(atlas.create_visualization_commands())
