# ============================================================================
# Arquivo: MuonEndcap.py
# Autora: Julya Pacheco Ferreira
# Descrição: Configuração da construção do Espectrômetro de Múons do ATLAS
#            na região dos Endcaps (A e C). Define as classes auxiliares
#            (Detector, MuonSampling, MuonChamberBuilder) e a função
#            getMuonEndcapCfg(left_side), que cria os volumes físicos e
#            sensíveis das câmaras MDT e TGC nos três wheels (EC1, EC2, EC3)
#            de cada lado. Retorna a lista de detectores configurados.
#            O parâmetro left_side=True configura o Endcap C (Z negativo) e
#            left_side=False o Endcap A (Z positivo).
# ============================================================================

__all__ = ["getMuonEndcapCfg"]

from GaugiKernel.constants import *
# CORRIGIDO: imports relativos dentro de geometry/python/v1/ (o modulo
# G4Kernel.DetectorConstruction nao existe no oficial; usamos os modulos v1)
from .PhysicalVolume    import *
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
    """Identificadores das samplings (wheels) de múon no Endcap."""
    MDT_EC_INNER = 1
    MDT_EC_MIDDLE = 2
    MDT_EC_OUTER = 3
    TGC_EC_INNER = 6
    TGC_EC_OUTER = 7

# ============================================================================
# Builder auxiliar para as câmaras de múon
# ============================================================================

class MuonChamberBuilder:
    """
    Encapsula a configuração de uma câmara de múon no Endcap (volume físico,
    sensitive, parâmetros de gás, shaper, ruído, amostras, pesos do filtro
    ótimo etc.).
    """
    def __init__(self, volume, bunch_start, bunch_end, sampling_start, 
                 CollectionKey, Detector, Sampling, Shaper, Noise, Samples,
                 OFWeightsEnergy, OFWeightsTime, GasMixture=None, OperatingVoltage=None,
                 GasPressure=None, SpatialResolution=None, ChamberStructure=None,
                 Layer=0, Station=0, ChamberType=0, Region=1,
                 EtaBins=1, PhiBins=8,
                 # NOVOS PARÂMETROS PARA TUBOS MDT
                 NofTubesPerChamber=200, NofLayers=4):
                 
        # --- Volume e parâmetros básicos de amostragem ------------------
        self.volume = volume
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

        # --- Identificação geométrica da câmara -------------------------
        self.Layer = Layer
        self.Station = Station
        self.ChamberType = ChamberType
        self.Region = Region
        self.BunchIdStart = bunch_start
        self.BunchIdEnd = bunch_end
        
        # NOVOS PARÂMETROS PARA TUBOS MDT
        self.NofTubesPerChamber = NofTubesPerChamber
        self.NofLayers = NofLayers
        
        # Criar um objeto 'sensitive' com os atributos necessários
        self.sensitive = type('obj', (object,), {
            'EtaBins': EtaBins,
            'PhiBins': PhiBins,
            # Adicionar informações dos tubos
            'NofTubesPerChamber': self.NofTubesPerChamber,
            'NofLayers': self.NofLayers
        })()
        
        # Criar estruturas sv.pv para compatibilidade
        self.sv = type('obj', (object,), {
            'pv': type('obj', (object,), {
                'R': volume.RMin if hasattr(volume, 'RMin') else 0.0
            })()
        })()
        
    def __repr__(self):
        side = "Endcap A" if self.Region == 1 else "Endcap C"
        return f"MuonChamberBuilder({self.Detector}, {self.Sampling}, Wheel={self.Station}, Tubes={self.NofTubesPerChamber}, Layers={self.NofLayers}, {side})"

# ============================================================================
# Função principal de configuração do Endcap
# ============================================================================

def getMuonEndcapCfg(left_side=False):
    """
    Configuração do Endcap do Espectrômetro de Múons do ATLAS
    
    Parâmetros:
    -----------
    left_side : bool
        Se True, configura o lado negativo (Endcap C) - Z negativo
        Se False, configura o lado positivo (Endcap A) - Z positivo
    """
    
    # IMPORTANTE: Para left_side=True (Endcap C), sign = -1
    # Para left_side=False (Endcap A), sign = +1
    sign = -1 if left_side else 1
    side_name = 'C' if left_side else 'A'
    region_code = 2 if left_side else 1
    
    # Configuração de tubos por câmara no endcap (valores realísticos do ATLAS)
    MDT_TUBES_PER_CHAMBER = 200   # ~200 tubos por câmara no endcap
    MDT_N_LAYERS = 4               # 4 camadas de tubos por roda
    
    # CORRIGIDO: LZT_PATH pode nao estar setado; usa fallback vazio
    basepath = os.environ.get('LZT_PATH', '') + '/geometry/data'

    # Posições base das rodas (em cm)
    z_wheel_inner = sign * 700*cm    # Endcap C: -700 cm, Endcap A: +700 cm
    z_wheel_middle = sign * 1050*cm  # Endcap C: -1050 cm, Endcap A: +1050 cm
    z_wheel_outer = sign * 1400*cm   # Endcap C: -1400 cm, Endcap A: +1400 cm
    
    # Offset para TGCs (ficam atrás dos MDTs) - MESMO sinal!
    tgc_offset = sign * 15*cm        # Endcap C: -15 cm, Endcap A: +15 cm

    # ============================================================
    # WHEEL 1 - MDT EC1 (Inner)
    # ============================================================
    mdt_ec1_pv = PhysicalVolume(
        Name               = "Muon::MDT_EC1::"+side_name,
        Plates             = Plates.Vertical,
        ChamberStructure   = "G4_Al",
        GasMixture         = "G4_Ar",
        NofLayers          = MDT_N_LAYERS,
        StructureThickness = 0.5*cm,
        GasGapThickness    = 3*cm,
        RMin               = 50*cm,
        RMax               = 850*cm,
        ZSize              = 4*(0.5*cm + 3*cm),
        X=0, Y=0, 
        Z = z_wheel_inner,
        Visualization = True,
        Color         = 'steelblue',
        IsMuonChamber = True,
        ChamberType   = 0,
        ElectronCut = 0.1*mm,
        PositronCut = 0.1*mm,
        GammaCut    = 0.1*mm,
        PhotonCut   = 0.1*mm,
        MuonCut     = 0.1*mm
    )

    # ============================================================
    # WHEEL 1 - TGC EC1 (Inner)
    # ============================================================
    tgc_ec1_pv = PhysicalVolume(
        Name               = "Muon::TGC_EC1::"+side_name,
        Plates             = Plates.Vertical,
        ChamberStructure   = "G4_Cu",
        GasMixture         = "G4_CO2",
        NofLayers          = 4,
        StructureThickness = 0.1*cm,
        GasGapThickness    = 0.28*cm,
        RMin               = 100*cm,
        RMax               = 850*cm,
        ZSize              = 4*(0.1*cm + 0.28*cm),
        X=0, Y=0, 
        Z = z_wheel_inner + tgc_offset,
        Visualization = True,
        Color         = 'lightsteelblue',
        IsMuonChamber = True,
        ChamberType   = 2,
        OperatingVoltage = 3.0e3,
        GasPressure     = 1.0,
        ElectronCut = 0.1*mm,
        PositronCut = 0.1*mm,
        GammaCut    = 0.1*mm,
        PhotonCut   = 0.1*mm,
        MuonCut     = 0.1*mm
    )

    # ============================================================
    # WHEEL 2 - MDT EC2 (Middle)
    # ============================================================
    mdt_ec2_pv = PhysicalVolume(
        Name               = "Muon::MDT_EC2::"+side_name,
        Plates             = Plates.Vertical,
        ChamberStructure   = "G4_Al",
        GasMixture         = "G4_Ar",
        NofLayers          = MDT_N_LAYERS,
        StructureThickness = 0.5*cm,
        GasGapThickness    = 3*cm,
        RMin               = 50*cm,
        RMax               = 850*cm,
        ZSize              = 3*(0.5*cm + 3*cm),
        X=0, Y=0, 
        Z = z_wheel_middle,
        Visualization = True,
        Color         = 'royalblue',
        IsMuonChamber = True,
        ChamberType   = 0,
        ElectronCut = 0.1*mm,
        PositronCut = 0.1*mm,
        GammaCut    = 0.1*mm,
        PhotonCut   = 0.1*mm,
        MuonCut     = 0.1*mm
    )

    # ============================================================
    # WHEEL 2 - TGC EC2 (Outer)
    # ============================================================
    tgc_ec2_pv = PhysicalVolume(
        Name               = "Muon::TGC_EC2::"+side_name,
        Plates             = Plates.Vertical,
        ChamberStructure   = "G4_Cu",
        GasMixture         = "G4_CO2",
        NofLayers          = 3,
        StructureThickness = 0.1*cm,
        GasGapThickness    = 0.28*cm,
        RMin               = 100*cm,
        RMax               = 850*cm,
        ZSize              = 3*(0.1*cm + 0.28*cm),
        X=0, Y=0, 
        Z = z_wheel_middle + tgc_offset,
        Visualization = True,
        Color         = 'cornflowerblue',
        IsMuonChamber = True,
        ChamberType   = 2,
        OperatingVoltage = 3.0e3,
        GasPressure     = 1.0,
        ElectronCut = 0.1*mm,
        PositronCut = 0.1*mm,
        GammaCut    = 0.1*mm,
        PhotonCut   = 0.1*mm,
        MuonCut     = 0.1*mm
    )

    # ============================================================
    # WHEEL 3 - MDT EC3 (Outer)
    # ============================================================
    mdt_ec3_pv = PhysicalVolume(
        Name               = "Muon::MDT_EC3::"+side_name,
        Plates             = Plates.Vertical,
        ChamberStructure   = "G4_Al",
        GasMixture         = "G4_Ar",
        NofLayers          = MDT_N_LAYERS,
        StructureThickness = 0.5*cm,
        GasGapThickness    = 3*cm,
        RMin               = 50*cm,
        RMax               = 850*cm,
        ZSize              = 3*(0.5*cm + 3*cm),
        X=0, Y=0, 
        Z = z_wheel_outer,
        Visualization = True,
        Color         = 'darkblue',
        IsMuonChamber = True,
        ChamberType   = 0,
        ElectronCut = 0.1*mm,
        PositronCut = 0.1*mm,
        GammaCut    = 0.1*mm,
        PhotonCut   = 0.1*mm,
        MuonCut     = 0.1*mm
    )

    # ============================================================
    # VOLUMES SENSÍVEIS
    # ============================================================
    mdt_ec1_sv = SensitiveVolume(mdt_ec1_pv, DeltaEta=0.01, DeltaPhi=pi/64)
    tgc_ec1_sv = SensitiveVolume(tgc_ec1_pv, DeltaEta=0.02, DeltaPhi=pi/32)
    mdt_ec2_sv = SensitiveVolume(mdt_ec2_pv, DeltaEta=0.01, DeltaPhi=pi/64)
    tgc_ec2_sv = SensitiveVolume(tgc_ec2_pv, DeltaEta=0.02, DeltaPhi=pi/32)
    mdt_ec3_sv = SensitiveVolume(mdt_ec3_pv, DeltaEta=0.01, DeltaPhi=pi/64)

    # ============================================================
    # CONFIGURAÇÃO DOS DETECTORES
    # ============================================================
    
    # MDT EC1 (Inner Wheel)
    mdt_ec1_det = MuonChamberBuilder(
        mdt_ec1_sv, 
        -21, 3, -2,
        CollectionKey   = "Collection_MDT_EC1_"+ side_name,
        Detector        = Detector.MDT,
        Sampling        = MuonSampling.MDT_EC_INNER,
        Shaper          = basepath + "/pulseMDT.dat",
        Noise           = 0.03*MeV,
        Samples         = 10,
        OFWeightsEnergy = [0.05, 0.1, 0.15, 0.2, 0.15, 0.1, 0.08, 0.07, 0.05, 0.05],
        OFWeightsTime   = [0.1, -2.5, 2.3, 0.05, 0.03, 0.02, 0.01, 0.01, 0.01, 0.01],
        GasMixture      = "Ar(93%)/CO2(7%)",
        OperatingVoltage = 3.1e3,
        GasPressure     = 3.0,
        SpatialResolution = 80.0,
        ChamberStructure = "Aluminum",
        Layer = 0,
        Station = 1,
        ChamberType = 0,
        Region = region_code,
        EtaBins = 8,
        PhiBins = 48,
        # PARÂMETROS DE TUBOS MDT
        NofTubesPerChamber = MDT_TUBES_PER_CHAMBER,
        NofLayers = MDT_N_LAYERS
    )

    # TGC EC1 (Inner Wheel)
    tgc_ec1_det = MuonChamberBuilder(
        tgc_ec1_sv, 
        -21, 3, -2,
        CollectionKey   = "Collection_TGC_EC1_"+ side_name,
        Detector        = Detector.TGC,
        Sampling        = MuonSampling.TGC_EC_INNER,
        Shaper          = basepath + "/pulseTGC.dat",
        Noise           = 0.001*MeV,
        Samples         = 3,
        OFWeightsEnergy = [0.25, 0.5, 0.25],
        OFWeightsTime   = [0.3, -2.0, 1.7],
        GasMixture      = "CO2(55%)/n-pentane(45%)",
        OperatingVoltage = 3.0e3,
        GasPressure     = 1.0,
        SpatialResolution = 10000.0,
        ChamberStructure = "Copper",
        Layer = 0,
        Station = 1,
        ChamberType = 2,
        Region = region_code,
        EtaBins = 6,
        PhiBins = 6
    )

    # MDT EC2 (Middle Wheel)
    mdt_ec2_det = MuonChamberBuilder(
        mdt_ec2_sv, 
        -21, 3, -2,
        CollectionKey   = "Collection_MDT_EC2_"+ side_name,
        Detector        = Detector.MDT,
        Sampling        = MuonSampling.MDT_EC_MIDDLE,
        Shaper          = basepath + "/pulseMDT.dat",
        Noise           = 0.03*MeV,
        Samples         = 10,
        OFWeightsEnergy = [0.05, 0.1, 0.15, 0.2, 0.15, 0.1, 0.08, 0.07, 0.05, 0.05],
        OFWeightsTime   = [0.1, -2.5, 2.3, 0.05, 0.03, 0.02, 0.01, 0.01, 0.01, 0.01],
        GasMixture      = "Ar(93%)/CO2(7%)",
        OperatingVoltage = 3.1e3,
        GasPressure     = 3.0,
        SpatialResolution = 80.0,
        ChamberStructure = "Aluminum",
        Layer = 0,
        Station = 2,
        ChamberType = 0,
        Region = region_code,
        EtaBins = 8,
        PhiBins = 48,
        # PARÂMETROS DE TUBOS MDT
        NofTubesPerChamber = MDT_TUBES_PER_CHAMBER,
        NofLayers = MDT_N_LAYERS
    )

    # TGC EC2 (Outer Wheel)
    tgc_ec2_det = MuonChamberBuilder(
        tgc_ec2_sv, 
        -21, 3, -2,
        CollectionKey   = "Collection_TGC_EC2_"+ side_name,
        Detector        = Detector.TGC,
        Sampling        = MuonSampling.TGC_EC_OUTER,
        Shaper          = basepath + "/pulseTGC.dat",
        Noise           = 0.001*MeV,
        Samples         = 3,
        OFWeightsEnergy = [0.25, 0.5, 0.25],
        OFWeightsTime   = [0.3, -2.0, 1.7],
        GasMixture      = "CO2(55%)/n-pentane(45%)",
        OperatingVoltage = 3.0e3,
        GasPressure     = 1.0,
        SpatialResolution = 10000.0,
        ChamberStructure = "Copper",
        Layer = 0,
        Station = 2,
        ChamberType = 2,
        Region = region_code,
        EtaBins = 6,
        PhiBins = 6
    )

    # MDT EC3 (Outer Wheel)
    mdt_ec3_det = MuonChamberBuilder(
        mdt_ec3_sv, 
        -21, 3, -2,
        CollectionKey   = "Collection_MDT_EC3_"+ side_name,
        Detector        = Detector.MDT,
        Sampling        = MuonSampling.MDT_EC_OUTER,
        Shaper          = basepath + "/pulseMDT.dat",
        Noise           = 0.03*MeV,
        Samples         = 10,
        OFWeightsEnergy = [0.05, 0.1, 0.15, 0.2, 0.15, 0.1, 0.08, 0.07, 0.05, 0.05],
        OFWeightsTime   = [0.1, -2.5, 2.3, 0.05, 0.03, 0.02, 0.01, 0.01, 0.01, 0.01],
        GasMixture      = "Ar(93%)/CO2(7%)",
        OperatingVoltage = 3.1e3,
        GasPressure     = 3.0,
        SpatialResolution = 80.0,
        ChamberStructure = "Aluminum",
        Layer = 0,
        Station = 3,
        ChamberType = 0,
        Region = region_code,
        EtaBins = 8,
        PhiBins = 48,
        # PARÂMETROS DE TUBOS MDT
        NofTubesPerChamber = MDT_TUBES_PER_CHAMBER,
        NofLayers = MDT_N_LAYERS
    )

    # Retorna a lista de detectores configurados para o Endcap
    return [mdt_ec1_det, tgc_ec1_det, mdt_ec2_det, tgc_ec2_det, mdt_ec3_det]
