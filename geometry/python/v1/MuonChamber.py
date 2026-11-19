
# ============================================================================
# Arquivo: MuonChamber.py
# Descrição: Configuração da construção do Espectrômetro de Múons do ATLAS
#            na região do Barrel. Define as classes auxiliares (Detector,
#            MuonSampling, MuonChamberBuilder) e a função getMuonBarrelCfg(),
#            que cria os volumes físicos e sensíveis das câmaras MDT (Inner,
#            Middle e Outer) e RPC (Middle e Outer), com suas propriedades
#            geométricas, de gás, de operação e de amostragem. Retorna a lista
#            de detectores configurados para o Barrel.
# ============================================================================

__all__ = ["getMuonBarrelCfg"]


#
# ATLAS Muon Spectrometer construction - Barrel
#

from GaugiKernel.constants import *

from .PhysicalVolume import *
from .SensitiveDetector import SensitiveDetector as SensitiveVolume

import numpy as np
import os


# ============================================================================
# Classes auxiliares de configuração
# ============================================================================

class Detector:
    """Códigos/nomes dos detectores de múon."""

    MDT = "MDT"
    RPC = "RPC"
    TGC = "TGC"


class MuonSampling:
    """Identificadores das samplings (camadas/estações de múon no Barrel)."""

    MDT_INNER = 1
    MDT_MIDDLE = 2
    MDT_OUTER = 3
    RPC_MIDDLE = 4
    RPC_OUTER = 5


# ============================================================================
# Builder auxiliar para as câmaras de múon
# ============================================================================

class MuonChamberBuilder:
    """
    Encapsula a configuração de uma câmara de múon.

    Mantém o SensitiveDetector associado à câmara e disponibiliza
    o PhysicalVolume através do método volume().
    """

    def __init__(
        self,
        volume,
        bunch_start,
        bunch_end,
        sampling_start,
        CollectionKey,
        Detector,
        Sampling,
        Shaper,
        Noise,
        Samples,
        OFWeightsEnergy,
        OFWeightsTime,
        GasMixture=None,
        OperatingVoltage=None,
        GasPressure=None,
        SpatialResolution=None,
        ChamberStructure=None,
        Layer=0,
        Station=0,
        ChamberType=0,
        Region=0,
        EtaBins=1,
        PhiBins=8,
        NofTubesPerLayer=308,
        NofLayers=4
    ):

        # Preserva o SensitiveDetector recebido.
        self.sensitive_detector = volume

        # Recupera o PhysicalVolume associado ao SensitiveDetector.
        self._volume = volume.pv if hasattr(volume, 'pv') else volume

        self.bunch_start = bunch_start
        self.bunch_end = bunch_end
        self.sampling_start = sampling_start

        self.CollectionKey = CollectionKey
        self.Detector = Detector
        self.Sampling = Sampling

        self.Shaper = Shaper
        self.Noise = Noise
        self.Samples = Samples

        self.OFWeightsEnergy = OFWeightsEnergy
        self.OFWeightsTime = OFWeightsTime

        self.GasMixture = GasMixture
        self.OperatingVoltage = OperatingVoltage
        self.GasPressure = GasPressure

        self.SpatialResolution = SpatialResolution
        self.ChamberStructure = ChamberStructure

        self.Layer = Layer
        self.Station = Station
        self.ChamberType = ChamberType
        self.Region = Region

        self.BunchIdStart = bunch_start
        self.BunchIdEnd = bunch_end

        self.NofTubesPerLayer = NofTubesPerLayer
        self.NofLayers = NofLayers

        # Informações de granularidade do detector.
        self.sensitive = type('obj', (object,), {
            'EtaBins': EtaBins,
            'PhiBins': PhiBins,
            'NofTubesPerLayer': self.NofTubesPerLayer,
            'NofLayers': self.NofLayers
        })()

        # Informações auxiliares da câmara.
        self.sv = type('obj', (object,), {
            'pv': type('obj', (object,), {
                'R': self._volume.RMin
                     if hasattr(self._volume, 'RMin') else 0.0
            })()
        })()

    def volume(self):
        """
        Retorna o PhysicalVolume da câmara.

        Interface utilizada por DetectorConstruction_v1.py.
        """
        return self._volume

    def __repr__(self):
        return (
            f"MuonChamberBuilder({self.Detector}, {self.Sampling}, "
            f"Tubes={self.NofTubesPerLayer}, "
            f"Layers={self.NofLayers})"
        )


# ============================================================================
# Função principal de configuração do Barrel
# ============================================================================

def getMuonBarrelCfg():
    """
    Configuração do Barrel do Espectrômetro de Múons do ATLAS.
    """

    # Caminho dos arquivos de configuração.
    basepath = os.environ.get('LZT_PATH', '') + '/geometry/data'

    MDT_TUBES_PER_LAYER = 308
    MDT_N_LAYERS = 4

    # ------------------------------------------------------------------------
    # VOLUMES FÍSICOS DAS CÂMARAS
    # ------------------------------------------------------------------------

    mdt_inner_pv = PhysicalVolume(
        Name="Muon::MDT_Inner",
        Plates=Plates.Horizontal,
        ChamberStructure="G4_Al",
        GasMixture="G4_Ar",
        NofLayers=MDT_N_LAYERS,
        StructureThickness=0.5*cm,
        GasGapThickness=30*cm,
        RMin=500*cm,
        RMax=550*cm,
        ZSize=7.0*m,
        X=0,
        Y=0,
        Z=0,
        Visualization=True,
        Color='steelblue',
        IsMuonChamber=True,
        ChamberType=0,
        MuonCut=0.1*mm
    )

    mdt_middle_pv = PhysicalVolume(
        Name="Muon::MDT_Middle",
        Plates=Plates.Horizontal,
        ChamberStructure="G4_Al",
        GasMixture="G4_Ar",
        NofLayers=MDT_N_LAYERS,
        StructureThickness=0.5*cm,
        GasGapThickness=30*cm,
        RMin=700*cm,
        RMax=750*cm,
        ZSize=7.0*m,
        X=0,
        Y=0,
        Z=0,
        Visualization=True,
        Color='royalblue',
        IsMuonChamber=True,
        ChamberType=0,
        MuonCut=0.1*mm
    )

    rpc_middle_pv = PhysicalVolume(
        Name="Muon::RPC_Middle",
        Plates=Plates.Horizontal,
        ChamberStructure="G4_POLYSTYRENE",
        GasMixture="G4_C2H2F4",
        NofLayers=4,
        StructureThickness=0.2*cm,
        GasGapThickness=0.2*cm,
        RMin=600*cm,
        RMax=650*cm,
        ZSize=7.0*m,
        X=0,
        Y=0,
        Z=0,
        Visualization=True,
        Color='lightblue',
        IsMuonChamber=True,
        ChamberType=1,
        OperatingVoltage=5.0e3,
        GasPressure=1.0,
        MuonCut=0.1*mm
    )

    mdt_outer_pv = PhysicalVolume(
        Name="Muon::MDT_Outer",
        Plates=Plates.Horizontal,
        ChamberStructure="G4_Al",
        GasMixture="G4_Ar",
        NofLayers=MDT_N_LAYERS,
        StructureThickness=0.5*cm,
        GasGapThickness=30*cm,
        RMin=900*cm,
        RMax=1000*cm,
        ZSize=7.0*m,
        X=0,
        Y=0,
        Z=0,
        Visualization=True,
        Color='darkblue',
        IsMuonChamber=True,
        ChamberType=0,
        MuonCut=0.1*mm
    )

    rpc_outer_pv = PhysicalVolume(
        Name="Muon::RPC_Outer",
        Plates=Plates.Horizontal,
        ChamberStructure="G4_POLYSTYRENE",
        GasMixture="G4_C2H2F4",
        NofLayers=2,
        StructureThickness=0.2*cm,
        GasGapThickness=0.2*cm,
        RMin=800*cm,
        RMax=850*cm,
        ZSize=7.0*m,
        X=0,
        Y=0,
        Z=0,
        Visualization=True,
        Color='skyblue',
        IsMuonChamber=True,
        ChamberType=1,
        OperatingVoltage=5.0e3,
        GasPressure=1.0,
        MuonCut=0.1*mm
    )

    # ------------------------------------------------------------------------
    # DEFINIR VOLUMES SENSÍVEIS
    # ------------------------------------------------------------------------

    mdt_inner_sv = SensitiveVolume(
        mdt_inner_pv,
        DeltaEta=0.01,
        DeltaPhi=pi/128
    )

    mdt_middle_sv = SensitiveVolume(
        mdt_middle_pv,
        DeltaEta=0.01,
        DeltaPhi=pi/128
    )

    mdt_outer_sv = SensitiveVolume(
        mdt_outer_pv,
        DeltaEta=0.01,
        DeltaPhi=pi/128
    )

    rpc_middle_sv = SensitiveVolume(
        rpc_middle_pv,
        DeltaEta=0.02,
        DeltaPhi=pi/32
    )

    rpc_outer_sv = SensitiveVolume(
        rpc_outer_pv,
        DeltaEta=0.02,
        DeltaPhi=pi/32
    )

    # ------------------------------------------------------------------------
    # CONFIGURAR OS DETECTORES
    # ------------------------------------------------------------------------

    mdt_inner_det = MuonChamberBuilder(
        mdt_inner_sv,
        -21, 3, -2,
        CollectionKey="Collection_MDT_Inner",
        Detector=Detector.MDT,
        Sampling=MuonSampling.MDT_INNER,
        Shaper=basepath + "/pulseMDT.dat",
        Noise=0.02*MeV,
        Samples=10,
        OFWeightsEnergy=[
            0.05, 0.1, 0.15, 0.2, 0.15,
            0.1, 0.08, 0.07, 0.05, 0.05
        ],
        OFWeightsTime=[
            0.1, -2.5, 2.3, 0.05, 0.03,
            0.02, 0.01, 0.01, 0.01, 0.01
        ],
        GasMixture="Ar(93%)/CO2(7%)",
        OperatingVoltage=3.1e3,
        GasPressure=3.0,
        SpatialResolution=80.0,
        ChamberStructure="Aluminum",
        Layer=0,
        Station=1,
        ChamberType=0,
        Region=0,
        EtaBins=16,
        PhiBins=32,
        NofTubesPerLayer=MDT_TUBES_PER_LAYER,
        NofLayers=MDT_N_LAYERS
    )

    mdt_middle_det = MuonChamberBuilder(
        mdt_middle_sv,
        -21, 3, -2,
        CollectionKey="Collection_MDT_Middle",
        Detector=Detector.MDT,
        Sampling=MuonSampling.MDT_MIDDLE,
        Shaper=basepath + "/pulseMDT.dat",
        Noise=0.02*MeV,
        Samples=10,
        OFWeightsEnergy=[
            0.05, 0.1, 0.15, 0.2, 0.15,
            0.1, 0.08, 0.07, 0.05, 0.05
        ],
        OFWeightsTime=[
            0.1, -2.5, 2.3, 0.05, 0.03,
            0.02, 0.01, 0.01, 0.01, 0.01
        ],
        GasMixture="Ar(93%)/CO2(7%)",
        OperatingVoltage=3.1e3,
        GasPressure=3.0,
        SpatialResolution=80.0,
        ChamberStructure="Aluminum",
        Layer=0,
        Station=2,
        ChamberType=0,
        Region=0,
        EtaBins=16,
        PhiBins=32,
        NofTubesPerLayer=MDT_TUBES_PER_LAYER,
        NofLayers=MDT_N_LAYERS
    )

    rpc_middle_det = MuonChamberBuilder(
        rpc_middle_sv,
        -21, 3, -2,
        CollectionKey="Collection_RPC_Middle",
        Detector=Detector.RPC,
        Sampling=MuonSampling.RPC_MIDDLE,
        Shaper=basepath + "/pulseRPC.dat",
        Noise=0.001*MeV,
        Samples=3,
        OFWeightsEnergy=[0.3, 0.4, 0.3],
        OFWeightsTime=[0.4, -1.0, 0.6],
        GasMixture="C2H2F4(94.7%)/C4H10(5%)/SF6(0.3%)",
        OperatingVoltage=5.0e3,
        GasPressure=1.0,
        SpatialResolution=10000.0,
        ChamberStructure="Bakelite",
        Layer=0,
        Station=1,
        ChamberType=1,
        Region=0,
        EtaBins=8,
        PhiBins=8
    )

    mdt_outer_det = MuonChamberBuilder(
        mdt_outer_sv,
        -21, 3, -2,
        CollectionKey="Collection_MDT_Outer",
        Detector=Detector.MDT,
        Sampling=MuonSampling.MDT_OUTER,
        Shaper=basepath + "/pulseMDT.dat",
        Noise=0.02*MeV,
        Samples=10,
        OFWeightsEnergy=[
            0.05, 0.1, 0.15, 0.2, 0.15,
            0.1, 0.08, 0.07, 0.05, 0.05
        ],
        OFWeightsTime=[
            0.1, -2.5, 2.3, 0.05, 0.03,
            0.02, 0.01, 0.01, 0.01, 0.01
        ],
        GasMixture="Ar(93%)/CO2(7%)",
        OperatingVoltage=3.1e3,
        GasPressure=3.0,
        SpatialResolution=80.0,
        ChamberStructure="Aluminum",
        Layer=0,
        Station=3,
        ChamberType=0,
        Region=0,
        EtaBins=16,
        PhiBins=32,
        NofTubesPerLayer=MDT_TUBES_PER_LAYER,
        NofLayers=MDT_N_LAYERS
    )

    rpc_outer_det = MuonChamberBuilder(
        rpc_outer_sv,
        -21, 3, -2,
        CollectionKey="Collection_RPC_Outer",
        Detector=Detector.RPC,
        Sampling=MuonSampling.RPC_OUTER,
        Shaper=basepath + "/pulseRPC.dat",
        Noise=0.001*MeV,
        Samples=3,
        OFWeightsEnergy=[0.3, 0.4, 0.3],
        OFWeightsTime=[0.4, -1.0, 0.6],
        GasMixture="C2H2F4(94.7%)/C4H10(5%)/SF6(0.3%)",
        OperatingVoltage=5.0e3,
        GasPressure=1.0,
        SpatialResolution=10000.0,
        ChamberStructure="Bakelite",
        Layer=0,
        Station=2,
        ChamberType=1,
        Region=0,
        EtaBins=8,
        PhiBins=8
    )

    # ------------------------------------------------------------------------
    # RETORNAR CONFIGURAÇÃO DO BARREL
    # ------------------------------------------------------------------------

    return [
        mdt_inner_det,
        mdt_middle_det,
        rpc_middle_det,
        mdt_outer_det,
        rpc_outer_det
    ]

