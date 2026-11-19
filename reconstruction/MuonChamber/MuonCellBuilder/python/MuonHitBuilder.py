# ============================================================================
# Arquivo: MuonHitBuilder.py
# Descrição: Builder responsável por montar a cadeia de criação de hits de
#            muon. Ele cria, para cada combinação de (detector, região,
#            camada/sampling), um MuonHitMaker com sua grade geométrica
#            (XBins/YBins/ZBins), faixa radial (RMin/RMax), faixa em Z
#            (ZMin/ZMax), range de Bunch Crossing IDs e duração do bunch
#            apropriados. Ao final, cria um MuonHitMerge para unificar todas
#            as coleções de hits em uma única coleção de saída
#            (OutputHitsKey).
#
#            ATENÇÃO: os RMin/RMax/ZMin/ZMax abaixo estão ALINHADOS com a
#            geometria definida em ATLAS/MuonChamber.py e ATLAS/MuonEndcap.py.
#            Se você mudar a geometria, mude estes valores também.
#
#            Cobertura:
#              - Barrel (Region=0):
#                  MDT_Inner   R=500-550  cm  |z|<=350  cm   (ZSize=7m)
#                  MDT_Middle  R=700-750  cm  |z|<=350  cm
#                  MDT_Outer   R=900-1000 cm  |z|<=350  cm
#                  RPC_Middle  R=600-650  cm  |z|<=350  cm
#                  RPC_Outer   R=800-850  cm  |z|<=350  cm
#              - Endcap A (Region=1, z>0):
#                  MDT_EC1     z= 700 cm  (ZSize=14 cm)   -> z in [693, 707]
#                  TGC_EC1     z= 700+15=715 cm           -> z in [707.4, 722.6]
#                  MDT_EC2     z=1050 cm  (ZSize=10.5 cm) -> z in [1044.75, 1055.25]
#                  TGC_EC2     z=1050+15=1065 cm          -> z in [1059.3, 1070.7]
#                  MDT_EC3     z=1400 cm  (ZSize=10.5 cm) -> z in [1394.75, 1405.25]
#              - Endcap C (Region=2, z<0): idem com sinal negativo.
# ============================================================================

__all__ = ["MuonHitBuilder"]

# --- Imports do framework ---------------------------------------------------
from GaugiKernel        import Logger, LoggingLevel
from GaugiKernel.macros import *
from G4Kernel import ComponentAccumulator
import numpy as np


class MuonHitBuilder(Logger):
    """
    Constrói e encadeia todos os MuonHitMaker (um por detector/região/
    camada) e o MuonHitMerge final. Herda de Logger para emitir mensagens
    padronizadas.
    """

    def __init__(self, name,
                 InputEventKey: str = "Events",
                 OutputHitsKey: str = "MuonHits",
                 HistogramPath: str = "/MuonHitBuilder",
                 OutputLevel: int = LoggingLevel.toC('INFO'),
                 ):

        # Inicializa o Logger
        Logger.__init__(self)

        # Lista interna que acumula todos os algoritmos de reconstrução
        self.__recoAlgs = []

        # Parâmetros de configuração
        self.HistogramPath        = HistogramPath
        self.OutputLevel          = OutputLevel
        self.OutputHitsKey        = OutputHitsKey
        self.OutputCollectionKeys = []    # chaves das coleções produzidas
        self.__name               = name

    def configure(self):
        """Configure the MuonHitBuilder with detector-specific settings"""

        MSG_INFO(self, "Configure MuonHitBuilder (geometry-aligned).")

        # Import here to avoid circular imports
        from MuonHitMaker import MuonHitMaker

        # ============================================
        # B A R R E L   (Region = 0)
        #
        # Geometria (MuonChamber.py):
        #   MDT_Inner  : R = 500-550  cm, ZSize = 7 m -> |z| <= 350 cm
        #   MDT_Middle : R = 700-750  cm, ZSize = 7 m -> |z| <= 350 cm
        #   MDT_Outer  : R = 900-1000 cm, ZSize = 7 m -> |z| <= 350 cm
        #   RPC_Middle : R = 600-650  cm, ZSize = 7 m -> |z| <= 350 cm
        #   RPC_Outer  : R = 800-850  cm, ZSize = 7 m -> |z| <= 350 cm
        # ============================================

        barrel_zmin_mm = -3500.0
        barrel_zmax_mm = +3500.0

        # -------------------------------------------------
        # MDT BARREL - INNER (R=500-550 cm, |Z|<=350 cm)
        # -------------------------------------------------
        mdt_inner_collection = "MDT_Inner_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT Inner Barrel and dump hits into %s collection",
            mdt_inner_collection
        )

        n_phi_inner = 16
        phi_bins_inner = np.linspace(-np.pi, np.pi, n_phi_inner + 1)
        avg_radius_inner = (500.0 + 550.0) / 2.0  # cm

        x_bins_inner = [avg_radius_inner * 10.0 * np.cos(phi) for phi in phi_bins_inner]  # mm
        y_bins_inner = [avg_radius_inner * 10.0 * np.sin(phi) for phi in phi_bins_inner]  # mm

        histogramPath = self.HistogramPath + '/MDT_Inner_Barrel'
        mdt_inner = MuonHitMaker(
            "MuonHitMaker_MDT_Inner_Barrel",
            OutputCollectionKey=mdt_inner_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_inner,
            YBins=y_bins_inner,
            ZBins=np.linspace(barrel_zmin_mm, barrel_zmax_mm, 33).tolist(),
            RMin=5000.0,   # mm  (500 cm)
            RMax=5500.0,   # mm  (550 cm)
            ZMin=barrel_zmin_mm,
            ZMax=barrel_zmax_mm,
            Detector="MDT",
            Sampling=1,
            Region=0,
            Station=1,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=25.0
        )
        self.__recoAlgs.append(mdt_inner)
        self.OutputCollectionKeys.append(mdt_inner_collection)

        # -------------------------------------------------
        # MDT BARREL - MIDDLE (R=700-750 cm, |Z|<=350 cm)
        # -------------------------------------------------
        mdt_middle_collection = "MDT_Middle_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT Middle Barrel and dump hits into %s collection",
            mdt_middle_collection
        )

        avg_radius_middle = (700.0 + 750.0) / 2.0  # cm

        x_bins_middle = [avg_radius_middle * 10.0 * np.cos(phi) for phi in phi_bins_inner]
        y_bins_middle = [avg_radius_middle * 10.0 * np.sin(phi) for phi in phi_bins_inner]

        histogramPath = self.HistogramPath + '/MDT_Middle_Barrel'
        mdt_middle = MuonHitMaker(
            "MuonHitMaker_MDT_Middle_Barrel",
            OutputCollectionKey=mdt_middle_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_middle,
            YBins=y_bins_middle,
            ZBins=np.linspace(barrel_zmin_mm, barrel_zmax_mm, 33).tolist(),
            RMin=7000.0,   # mm  (700 cm)
            RMax=7500.0,   # mm  (750 cm)
            ZMin=barrel_zmin_mm,
            ZMax=barrel_zmax_mm,
            Detector="MDT",
            Sampling=2,
            Region=0,
            Station=2,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=25.0
        )
        self.__recoAlgs.append(mdt_middle)
        self.OutputCollectionKeys.append(mdt_middle_collection)

        # -------------------------------------------------
        # MDT BARREL - OUTER (R=900-1000 cm, |Z|<=350 cm)
        # -------------------------------------------------
        mdt_outer_collection = "MDT_Outer_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT Outer Barrel and dump hits into %s collection",
            mdt_outer_collection
        )

        avg_radius_outer = (900.0 + 1000.0) / 2.0  # cm

        x_bins_outer = [avg_radius_outer * 10.0 * np.cos(phi) for phi in phi_bins_inner]
        y_bins_outer = [avg_radius_outer * 10.0 * np.sin(phi) for phi in phi_bins_inner]

        histogramPath = self.HistogramPath + '/MDT_Outer_Barrel'
        mdt_outer = MuonHitMaker(
            "MuonHitMaker_MDT_Outer_Barrel",
            OutputCollectionKey=mdt_outer_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_outer,
            YBins=y_bins_outer,
            ZBins=np.linspace(barrel_zmin_mm, barrel_zmax_mm, 33).tolist(),
            RMin=9000.0,   # mm  (900 cm)
            RMax=10000.0,  # mm  (1000 cm)
            ZMin=barrel_zmin_mm,
            ZMax=barrel_zmax_mm,
            Detector="MDT",
            Sampling=3,
            Region=0,
            Station=3,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=25.0
        )
        self.__recoAlgs.append(mdt_outer)
        self.OutputCollectionKeys.append(mdt_outer_collection)

        # -------------------------------------------------
        # RPC BARREL - MIDDLE (R=600-650 cm, |Z|<=350 cm)
        # -------------------------------------------------
        rpc_middle_collection = "RPC_Middle_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for RPC Middle Barrel and dump hits into %s collection",
            rpc_middle_collection
        )

        n_phi_rpc = 8
        phi_bins_rpc = np.linspace(-np.pi, np.pi, n_phi_rpc + 1)
        avg_radius_rpc_middle = (600.0 + 650.0) / 2.0  # cm

        x_bins_rpc_middle = [avg_radius_rpc_middle * 10.0 * np.cos(phi) for phi in phi_bins_rpc]
        y_bins_rpc_middle = [avg_radius_rpc_middle * 10.0 * np.sin(phi) for phi in phi_bins_rpc]

        histogramPath = self.HistogramPath + '/RPC_Middle_Barrel'
        rpc_middle = MuonHitMaker(
            "MuonHitMaker_RPC_Middle_Barrel",
            OutputCollectionKey=rpc_middle_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_rpc_middle,
            YBins=y_bins_rpc_middle,
            ZBins=np.linspace(barrel_zmin_mm, barrel_zmax_mm, 33).tolist(),
            RMin=6000.0,   # mm  (600 cm)
            RMax=6500.0,   # mm  (650 cm)
            ZMin=barrel_zmin_mm,
            ZMax=barrel_zmax_mm,
            Detector="RPC",
            Sampling=4,
            Region=0,
            Station=1,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=25.0
        )
        self.__recoAlgs.append(rpc_middle)
        self.OutputCollectionKeys.append(rpc_middle_collection)

        # -------------------------------------------------
        # RPC BARREL - OUTER (R=800-850 cm, |Z|<=350 cm)
        # -------------------------------------------------
        rpc_outer_collection = "RPC_Outer_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for RPC Outer Barrel and dump hits into %s collection",
            rpc_outer_collection
        )

        avg_radius_rpc_outer = (800.0 + 850.0) / 2.0  # cm

        x_bins_rpc_outer = [avg_radius_rpc_outer * 10.0 * np.cos(phi) for phi in phi_bins_rpc]
        y_bins_rpc_outer = [avg_radius_rpc_outer * 10.0 * np.sin(phi) for phi in phi_bins_rpc]

        histogramPath = self.HistogramPath + '/RPC_Outer_Barrel'
        rpc_outer = MuonHitMaker(
            "MuonHitMaker_RPC_Outer_Barrel",
            OutputCollectionKey=rpc_outer_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_rpc_outer,
            YBins=y_bins_rpc_outer,
            ZBins=np.linspace(barrel_zmin_mm, barrel_zmax_mm, 33).tolist(),
            RMin=8000.0,   # mm  (800 cm)
            RMax=8500.0,   # mm  (850 cm)
            ZMin=barrel_zmin_mm,
            ZMax=barrel_zmax_mm,
            Detector="RPC",
            Sampling=5,
            Region=0,
            Station=2,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=25.0
        )
        self.__recoAlgs.append(rpc_outer)
        self.OutputCollectionKeys.append(rpc_outer_collection)

        # ============================================
        # E N D C A P   A   (Region = 1, Z > 0)
        #
        # Geometria (MuonEndcap.py):
        #   MDT_EC1 z = +700 cm,  ZSize = 14 cm    -> z in [693,   707]     cm
        #   TGC_EC1 z = +715 cm,  ZSize = 15.2 cm  -> z in [707.4, 722.6]   cm
        #   MDT_EC2 z = +1050 cm, ZSize = 10.5 cm  -> z in [1044.75, 1055.25] cm
        #   TGC_EC2 z = +1065 cm, ZSize = 11.4 cm  -> z in [1059.3, 1070.7] cm
        #   MDT_EC3 z = +1400 cm, ZSize = 10.5 cm  -> z in [1394.75, 1405.25] cm
        # ============================================

        # Parâmetros comuns para Endcaps
        n_phi_ec = 16
        n_r_ec = 10
        phi_bins_ec = np.linspace(-np.pi, np.pi, n_phi_ec + 1)
        r_bins_ec = np.linspace(50.0, 850.0, n_r_ec + 1)  # cm

        # Gera os pontos (x,y) para MDT endcap
        x_bins_ec = []
        y_bins_ec = []
        for r in r_bins_ec:
            for phi in phi_bins_ec:
                x_bins_ec.append(r * 10.0 * np.cos(phi))
                y_bins_ec.append(r * 10.0 * np.sin(phi))
        x_bins_ec = sorted(list(set(x_bins_ec)))
        y_bins_ec = sorted(list(set(y_bins_ec)))

        # Parâmetros comuns para TGC (menor granularidade)
        n_phi_tgc = 8
        n_r_tgc = 5
        phi_bins_tgc = np.linspace(-np.pi, np.pi, n_phi_tgc + 1)
        r_bins_tgc = np.linspace(100.0, 850.0, n_r_tgc + 1)  # cm

        x_bins_tgc = []
        y_bins_tgc = []
        for r in r_bins_tgc:
            for phi in phi_bins_tgc:
                x_bins_tgc.append(r * 10.0 * np.cos(phi))
                y_bins_tgc.append(r * 10.0 * np.sin(phi))
        x_bins_tgc = sorted(list(set(x_bins_tgc)))
        y_bins_tgc = sorted(list(set(y_bins_tgc)))

        # -------------------------------------------------
        # MDT ENDCAP A - INNER (z em [693, 707] cm)
        # -------------------------------------------------
        mdt_ec_a1_collection = "MDT_EndcapA_Inner_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapA Inner and dump hits into %s collection",
            mdt_ec_a1_collection
        )

        histogramPath = self.HistogramPath + '/MDT_EndcapA_Inner'
        mdt_ec_a1 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapA_Inner",
            OutputCollectionKey=mdt_ec_a1_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec,
            YBins=y_bins_ec,
            ZBins=np.linspace(6930.0, 7070.0, 9).tolist(),  # mm
            RMin=500.0,     # mm  -- atenção: no MuonHitMaker, RMin/RMax são em cm
            RMax=8500.0,    # mm
            ZMin=6930.0,    # mm
            ZMax=7070.0,    # mm
            Detector="MDT",
            Sampling=1,
            Region=1,
            Station=1,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=35.0
        )
        self.__recoAlgs.append(mdt_ec_a1)
        self.OutputCollectionKeys.append(mdt_ec_a1_collection)

        # -------------------------------------------------
        # TGC ENDCAP A - INNER (z em [707.4, 722.6] cm)
        # -------------------------------------------------
        tgc_ec_a1_collection = "TGC_EndcapA_Inner_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for TGC EndcapA Inner and dump hits into %s collection",
            tgc_ec_a1_collection
        )

        histogramPath = self.HistogramPath + '/TGC_EndcapA_Inner'
        tgc_ec_a1 = MuonHitMaker(
            "MuonHitMaker_TGC_EndcapA_Inner",
            OutputCollectionKey=tgc_ec_a1_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_tgc,
            YBins=y_bins_tgc,
            ZBins=np.linspace(7074.0, 7226.0, 7).tolist(),  # mm
            RMin=1000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=7074.0,    # mm
            ZMax=7226.0,    # mm
            Detector="TGC",
            Sampling=6,
            Region=1,
            Station=1,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=35.0
        )
        self.__recoAlgs.append(tgc_ec_a1)
        self.OutputCollectionKeys.append(tgc_ec_a1_collection)

        # -------------------------------------------------
        # MDT ENDCAP A - MIDDLE (z em [1044.75, 1055.25] cm)
        # -------------------------------------------------
        mdt_ec_a2_collection = "MDT_EndcapA_Middle_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapA Middle and dump hits into %s collection",
            mdt_ec_a2_collection
        )

        histogramPath = self.HistogramPath + '/MDT_EndcapA_Middle'
        mdt_ec_a2 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapA_Middle",
            OutputCollectionKey=mdt_ec_a2_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec,
            YBins=y_bins_ec,
            ZBins=np.linspace(10447.5, 10552.5, 9).tolist(),  # mm
            RMin=500.0,     # mm
            RMax=8500.0,    # mm
            ZMin=10447.5,   # mm
            ZMax=10552.5,   # mm
            Detector="MDT",
            Sampling=2,
            Region=1,
            Station=2,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=40.0
        )
        self.__recoAlgs.append(mdt_ec_a2)
        self.OutputCollectionKeys.append(mdt_ec_a2_collection)

        # -------------------------------------------------
        # TGC ENDCAP A - OUTER (z em [1059.3, 1070.7] cm)
        # -------------------------------------------------
        tgc_ec_a2_collection = "TGC_EndcapA_Outer_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for TGC EndcapA Outer and dump hits into %s collection",
            tgc_ec_a2_collection
        )

        histogramPath = self.HistogramPath + '/TGC_EndcapA_Outer'
        tgc_ec_a2 = MuonHitMaker(
            "MuonHitMaker_TGC_EndcapA_Outer",
            OutputCollectionKey=tgc_ec_a2_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_tgc,
            YBins=y_bins_tgc,
            ZBins=np.linspace(10593.0, 10707.0, 7).tolist(),  # mm
            RMin=1000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=10593.0,   # mm
            ZMax=10707.0,   # mm
            Detector="TGC",
            Sampling=7,
            Region=1,
            Station=2,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=40.0
        )
        self.__recoAlgs.append(tgc_ec_a2)
        self.OutputCollectionKeys.append(tgc_ec_a2_collection)

        # -------------------------------------------------
        # MDT ENDCAP A - OUTER (z em [1394.75, 1405.25] cm)
        # -------------------------------------------------
        mdt_ec_a3_collection = "MDT_EndcapA_Outer_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapA Outer and dump hits into %s collection",
            mdt_ec_a3_collection
        )

        histogramPath = self.HistogramPath + '/MDT_EndcapA_Outer'
        mdt_ec_a3 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapA_Outer",
            OutputCollectionKey=mdt_ec_a3_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec,
            YBins=y_bins_ec,
            ZBins=np.linspace(13947.5, 14052.5, 9).tolist(),  # mm
            RMin=500.0,     # mm
            RMax=8500.0,    # mm
            ZMin=13947.5,   # mm
            ZMax=14052.5,   # mm
            Detector="MDT",
            Sampling=3,
            Region=1,
            Station=3,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=50.0
        )
        self.__recoAlgs.append(mdt_ec_a3)
        self.OutputCollectionKeys.append(mdt_ec_a3_collection)

        # ============================================
        # E N D C A P   C   (Region = 2, Z < 0)
        # ============================================

        # -------------------------------------------------
        # MDT ENDCAP C - INNER (z em [-707, -693] cm)
        # -------------------------------------------------
        mdt_ec_c1_collection = "MDT_EndcapC_Inner_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapC Inner and dump hits into %s collection",
            mdt_ec_c1_collection
        )

        histogramPath = self.HistogramPath + '/MDT_EndcapC_Inner'
        mdt_ec_c1 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapC_Inner",
            OutputCollectionKey=mdt_ec_c1_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec,
            YBins=y_bins_ec,
            ZBins=np.linspace(-7070.0, -6930.0, 9).tolist(),  # mm
            RMin=500.0,     # mm
            RMax=8500.0,    # mm
            ZMin=-7070.0,   # mm
            ZMax=-6930.0,   # mm
            Detector="MDT",
            Sampling=1,
            Region=2,
            Station=1,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=35.0
        )
        self.__recoAlgs.append(mdt_ec_c1)
        self.OutputCollectionKeys.append(mdt_ec_c1_collection)

        # -------------------------------------------------
        # TGC ENDCAP C - INNER (z em [-722.6, -707.4] cm)
        # -------------------------------------------------
        tgc_ec_c1_collection = "TGC_EndcapC_Inner_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for TGC EndcapC Inner and dump hits into %s collection",
            tgc_ec_c1_collection
        )

        histogramPath = self.HistogramPath + '/TGC_EndcapC_Inner'
        tgc_ec_c1 = MuonHitMaker(
            "MuonHitMaker_TGC_EndcapC_Inner",
            OutputCollectionKey=tgc_ec_c1_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_tgc,
            YBins=y_bins_tgc,
            ZBins=np.linspace(-7226.0, -7074.0, 7).tolist(),  # mm
            RMin=1000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=-7226.0,   # mm
            ZMax=-7074.0,   # mm
            Detector="TGC",
            Sampling=6,
            Region=2,
            Station=1,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=35.0
        )
        self.__recoAlgs.append(tgc_ec_c1)
        self.OutputCollectionKeys.append(tgc_ec_c1_collection)

        # -------------------------------------------------
        # MDT ENDCAP C - MIDDLE (z em [-1055.25, -1044.75] cm)
        # -------------------------------------------------
        mdt_ec_c2_collection = "MDT_EndcapC_Middle_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapC Middle and dump hits into %s collection",
            mdt_ec_c2_collection
        )

        histogramPath = self.HistogramPath + '/MDT_EndcapC_Middle'
        mdt_ec_c2 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapC_Middle",
            OutputCollectionKey=mdt_ec_c2_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec,
            YBins=y_bins_ec,
            ZBins=np.linspace(-10552.5, -10447.5, 9).tolist(),  # mm
            RMin=500.0,     # mm
            RMax=8500.0,    # mm
            ZMin=-10552.5,  # mm
            ZMax=-10447.5,  # mm
            Detector="MDT",
            Sampling=2,
            Region=2,
            Station=2,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=40.0
        )
        self.__recoAlgs.append(mdt_ec_c2)
        self.OutputCollectionKeys.append(mdt_ec_c2_collection)

        # -------------------------------------------------
        # TGC ENDCAP C - OUTER (z em [-1070.7, -1059.3] cm)
        # -------------------------------------------------
        tgc_ec_c2_collection = "TGC_EndcapC_Outer_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for TGC EndcapC Outer and dump hits into %s collection",
            tgc_ec_c2_collection
        )

        histogramPath = self.HistogramPath + '/TGC_EndcapC_Outer'
        tgc_ec_c2 = MuonHitMaker(
            "MuonHitMaker_TGC_EndcapC_Outer",
            OutputCollectionKey=tgc_ec_c2_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_tgc,
            YBins=y_bins_tgc,
            ZBins=np.linspace(-10707.0, -10593.0, 7).tolist(),  # mm
            RMin=1000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=-10707.0,  # mm
            ZMax=-10593.0,  # mm
            Detector="TGC",
            Sampling=7,
            Region=2,
            Station=2,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=40.0
        )
        self.__recoAlgs.append(tgc_ec_c2)
        self.OutputCollectionKeys.append(tgc_ec_c2_collection)

        # -------------------------------------------------
        # MDT ENDCAP C - OUTER (z em [-1405.25, -1394.75] cm)
        # -------------------------------------------------
        mdt_ec_c3_collection = "MDT_EndcapC_Outer_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapC Outer and dump hits into %s collection",
            mdt_ec_c3_collection
        )

        histogramPath = self.HistogramPath + '/MDT_EndcapC_Outer'
        mdt_ec_c3 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapC_Outer",
            OutputCollectionKey=mdt_ec_c3_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec,
            YBins=y_bins_ec,
            ZBins=np.linspace(-14052.5, -13947.5, 9).tolist(),  # mm
            RMin=500.0,     # mm
            RMax=8500.0,    # mm
            ZMin=-14052.5,  # mm
            ZMax=-13947.5,  # mm
            Detector="MDT",
            Sampling=3,
            Region=2,
            Station=3,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=50.0
        )
        self.__recoAlgs.append(mdt_ec_c3)
        self.OutputCollectionKeys.append(mdt_ec_c3_collection)

        # ============================================
        # M E R G E   A L G O R I T H M
        # ============================================

        MSG_INFO(
            self,
            "Create MuonHitMerge and dump all hit collections into %s container",
            self.OutputHitsKey
        )

        try:
            from MuonHitMerge import MuonHitMerge
            mergeAlg = MuonHitMerge(
                "MuonHitMerge",
                InputCollectionKeys=self.OutputCollectionKeys,
                OutputHitsKey=self.OutputHitsKey,
                OutputLevel=self.OutputLevel
            )
            self.__recoAlgs.append(mergeAlg)
            MSG_INFO(self, "MuonHitMerge created successfully")
        except ImportError as e:
            MSG_WARNING(self, f"MuonHitMerge not available: {e}. Continuing without merge...")
        except Exception as e:
            MSG_WARNING(self, f"Error creating MuonHitMerge: {e}")

    def merge(self, acc: ComponentAccumulator):
        """
        Obtains the detector from the ComponentAccumulator and appends
        all the hit makers required by the accumulator's detector.

        Parameters
        ----------
        acc : ComponentAccumulator
            Accumulator to merge with
        """
        # Garante que configure() foi rodado antes de adicionar ao acc
        self.configure()

        # Adiciona cada algoritmo ao acumulador
        for reco in self.__recoAlgs:
            acc += reco

        MSG_INFO(self, f"Added {len(self.__recoAlgs)} algorithms to accumulator")

    def __add__(self, other):
        """Support for adding algorithms directly"""
        # Aceita tanto uma lista de algoritmos quanto um único algoritmo
        if isinstance(other, list):
            self.__recoAlgs.extend(other)
        else:
            self.__recoAlgs.append(other)
        return self
