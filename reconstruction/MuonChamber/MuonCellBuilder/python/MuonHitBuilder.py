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
#            Atualmente cobre:
#              - Barrel (Region=0):      MDT Inner/Middle/Outer,
#                                        RPC Inner/Outer
#              - Endcap A (Region=1):    MDT Inner/Middle/Outer,
#                                        TGC Inner/Outer
#              - Endcap C (Region=2):    MDT Inner/Middle/Outer,
#                                        TGC Inner/Outer
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

        MSG_INFO(self, "Configure MuonHitBuilder.")

        # Import here to avoid circular imports
        from MuonHitMaker import MuonHitMaker

        # ============================================
        # B A R R E L   (Region = 0)
        # ============================================

        # -------------------------------------------------
        # MDT BARREL - INNER (R=350-400cm, Z=-800-800cm)
        # -------------------------------------------------
        mdt_inner_collection = "MDT_Inner_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT Inner Barrel and dump hits into %s collection",
            mdt_inner_collection
        )

        # Divide o círculo em 16 setores phi para gerar a grade em X,Y
        n_phi_inner = 16
        phi_bins = np.linspace(-np.pi, np.pi, n_phi_inner + 1)
        avg_radius_inner = (350.0 + 400.0) / 2.0  # cm

        # CORRIGIDO: y_bins agora usa sin(phi)
        # x = r·cos(φ), y = r·sin(φ) — coordenadas cartesianas da grade
        x_bins_inner = [avg_radius_inner * 10.0 * np.cos(phi) for phi in phi_bins]  # mm
        y_bins_inner = [avg_radius_inner * 10.0 * np.sin(phi) for phi in phi_bins]  # mm - CORRIGIDO!

        # Cria o MuonHitMaker do MDT Inner Barrel
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
            ZBins=np.linspace(-8000.0, 8000.0, 33).tolist(),  # mm
            RMin=3500.0,   # mm
            RMax=4000.0,   # mm
            ZMin=-8000.0,  # mm
            ZMax=8000.0,   # mm
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
        # MDT BARREL - MIDDLE (R=500-550cm, Z=-800-800cm)
        # -------------------------------------------------
        mdt_middle_collection = "MDT_Middle_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT Middle Barrel and dump hits into %s collection",
            mdt_middle_collection
        )

        avg_radius_middle = (500.0 + 550.0) / 2.0  # cm

        # Grade cartesiana (x = r·cos φ, y = r·sin φ)
        # CORRIGIDO: y_bins agora usa sin(phi)
        x_bins_middle = [avg_radius_middle * 10.0 * np.cos(phi) for phi in phi_bins]
        y_bins_middle = [avg_radius_middle * 10.0 * np.sin(phi) for phi in phi_bins]  # CORRIGIDO!

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
            ZBins=np.linspace(-8000.0, 8000.0, 33).tolist(),
            RMin=5000.0,   # mm
            RMax=5500.0,   # mm
            ZMin=-8000.0,  # mm
            ZMax=8000.0,   # mm
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
        # MDT BARREL - OUTER (R=700-750cm, Z=-800-800cm)
        # -------------------------------------------------
        mdt_outer_collection = "MDT_Outer_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT Outer Barrel and dump hits into %s collection",
            mdt_outer_collection
        )

        avg_radius_outer = (700.0 + 750.0) / 2.0  # cm

        # Grade cartesiana (x = r·cos φ, y = r·sin φ)
        # CORRIGIDO: y_bins agora usa sin(phi)
        x_bins_outer = [avg_radius_outer * 10.0 * np.cos(phi) for phi in phi_bins]
        y_bins_outer = [avg_radius_outer * 10.0 * np.sin(phi) for phi in phi_bins]  # CORRIGIDO!

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
            ZBins=np.linspace(-8000.0, 8000.0, 33).tolist(),
            RMin=7000.0,   # mm
            RMax=7500.0,   # mm
            ZMin=-8000.0,  # mm
            ZMax=8000.0,   # mm
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
        # RPC BARREL - INNER (R=420-450cm, Z=-800-800cm)
        # -------------------------------------------------
        rpc_inner_collection = "RPC_Inner_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for RPC Inner Barrel and dump hits into %s collection",
            rpc_inner_collection
        )

        # RPC usa apenas 8 setores phi (granularidade menor)
        n_phi_rpc = 8
        phi_bins_rpc = np.linspace(-np.pi, np.pi, n_phi_rpc + 1)
        avg_radius_rpc_inner = (420.0 + 450.0) / 2.0  # cm

        # Grade cartesiana (x = r·cos φ, y = r·sin φ)
        # CORRIGIDO: y_bins agora usa sin(phi)
        x_bins_rpc_inner = [avg_radius_rpc_inner * 10.0 * np.cos(phi) for phi in phi_bins_rpc]
        y_bins_rpc_inner = [avg_radius_rpc_inner * 10.0 * np.sin(phi) for phi in phi_bins_rpc]  # CORRIGIDO!

        histogramPath = self.HistogramPath + '/RPC_Inner_Barrel'
        rpc_inner = MuonHitMaker(
            "MuonHitMaker_RPC_Inner_Barrel",
            OutputCollectionKey=rpc_inner_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_rpc_inner,
            YBins=y_bins_rpc_inner,
            ZBins=np.linspace(-8000.0, 8000.0, 33).tolist(),
            RMin=4200.0,   # mm
            RMax=4500.0,   # mm
            ZMin=-8000.0,  # mm
            ZMax=8000.0,   # mm
            Detector="RPC",
            Sampling=4,
            Region=0,
            Station=1,
            BunchIdStart=-21,
            BunchIdEnd=21,
            BunchDuration=25.0
        )
        self.__recoAlgs.append(rpc_inner)
        self.OutputCollectionKeys.append(rpc_inner_collection)

        # -------------------------------------------------
        # RPC BARREL - OUTER (R=600-650cm, Z=-800-800cm)
        # -------------------------------------------------
        rpc_outer_collection = "RPC_Outer_Barrel_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for RPC Outer Barrel and dump hits into %s collection",
            rpc_outer_collection
        )

        avg_radius_rpc_outer = (600.0 + 650.0) / 2.0  # cm

        # Grade cartesiana (x = r·cos φ, y = r·sin φ)
        # CORRIGIDO: y_bins agora usa sin(phi)
        x_bins_rpc_outer = [avg_radius_rpc_outer * 10.0 * np.cos(phi) for phi in phi_bins_rpc]
        y_bins_rpc_outer = [avg_radius_rpc_outer * 10.0 * np.sin(phi) for phi in phi_bins_rpc]  # CORRIGIDO!

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
            ZBins=np.linspace(-8000.0, 8000.0, 33).tolist(),
            RMin=6000.0,   # mm
            RMax=6500.0,   # mm
            ZMin=-8000.0,  # mm
            ZMax=8000.0,   # mm
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
        # ============================================

        # Parâmetros comuns para Endcaps
        # phi: 16 setores, R: 10 anéis entre 200 e 850 cm
        n_phi_ec = 16
        n_r_ec = 10
        phi_bins_ec = np.linspace(-np.pi, np.pi, n_phi_ec + 1)
        r_bins_ec = np.linspace(200.0, 850.0, n_r_ec + 1)  # cm

        # -------------------------------------------------
        # MDT ENDCAP A - INNER (Z=1150-1250 cm)
        # -------------------------------------------------
        mdt_ec_a1_collection = "MDT_EndcapA_Inner_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapA Inner and dump hits into %s collection",
            mdt_ec_a1_collection
        )

        # Gera a grade combinando raios (r_bins_ec) e ângulos (phi_bins_ec)
        # Cada ponto (r, φ) vira (x, y) = (r·cos φ, r·sin φ)
        x_bins_ec_a1 = []
        y_bins_ec_a1 = []
        for r in r_bins_ec:
            for phi in phi_bins_ec:
                x_bins_ec_a1.append(r * 10.0 * np.cos(phi))  # mm
                y_bins_ec_a1.append(r * 10.0 * np.sin(phi))  # mm - CORRIGIDO!
        x_bins_ec_a1 = sorted(list(set(x_bins_ec_a1)))
        y_bins_ec_a1 = sorted(list(set(y_bins_ec_a1)))

        histogramPath = self.HistogramPath + '/MDT_EndcapA_Inner'
        mdt_ec_a1 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapA_Inner",
            OutputCollectionKey=mdt_ec_a1_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec_a1,
            YBins=y_bins_ec_a1,
            ZBins=np.linspace(11500.0, 12500.0, 21).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=11500.0,   # mm
            ZMax=12500.0,   # mm
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
        # TGC ENDCAP A - INNER (Z=1265-1295 cm)
        # -------------------------------------------------
        tgc_ec_a1_collection = "TGC_EndcapA_Inner_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for TGC EndcapA Inner and dump hits into %s collection",
            tgc_ec_a1_collection
        )

        # TGC usa apenas 8 setores phi e 5 anéis em R
        n_phi_tgc = 8
        n_r_tgc = 5
        phi_bins_tgc = np.linspace(-np.pi, np.pi, n_phi_tgc + 1)
        r_bins_tgc = np.linspace(200.0, 850.0, n_r_tgc + 1)  # cm

        x_bins_tgc_a1 = []
        y_bins_tgc_a1 = []
        for r in r_bins_tgc:
            for phi in phi_bins_tgc:
                x_bins_tgc_a1.append(r * 10.0 * np.cos(phi))
                y_bins_tgc_a1.append(r * 10.0 * np.sin(phi))  # CORRIGIDO!
        x_bins_tgc_a1 = sorted(list(set(x_bins_tgc_a1)))
        y_bins_tgc_a1 = sorted(list(set(y_bins_tgc_a1)))

        histogramPath = self.HistogramPath + '/TGC_EndcapA_Inner'
        tgc_ec_a1 = MuonHitMaker(
            "MuonHitMaker_TGC_EndcapA_Inner",
            OutputCollectionKey=tgc_ec_a1_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_tgc_a1,
            YBins=y_bins_tgc_a1,
            ZBins=np.linspace(12650.0, 12950.0, 11).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=12650.0,   # mm
            ZMax=12950.0,   # mm
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
        # MDT ENDCAP A - MIDDLE (Z=1340-1380 cm)
        # -------------------------------------------------
        mdt_ec_a2_collection = "MDT_EndcapA_Middle_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapA Middle and dump hits into %s collection",
            mdt_ec_a2_collection
        )

        x_bins_ec_a2 = []
        y_bins_ec_a2 = []
        for r in r_bins_ec:
            for phi in phi_bins_ec:
                x_bins_ec_a2.append(r * 10.0 * np.cos(phi))
                y_bins_ec_a2.append(r * 10.0 * np.sin(phi))  # CORRIGIDO!
        x_bins_ec_a2 = sorted(list(set(x_bins_ec_a2)))
        y_bins_ec_a2 = sorted(list(set(y_bins_ec_a2)))

        histogramPath = self.HistogramPath + '/MDT_EndcapA_Middle'
        mdt_ec_a2 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapA_Middle",
            OutputCollectionKey=mdt_ec_a2_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec_a2,
            YBins=y_bins_ec_a2,
            ZBins=np.linspace(13400.0, 13800.0, 21).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=13400.0,   # mm
            ZMax=13800.0,   # mm
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
        # TGC ENDCAP A - OUTER (Z=1425-1455 cm)
        # -------------------------------------------------
        tgc_ec_a2_collection = "TGC_EndcapA_Outer_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for TGC EndcapA Outer and dump hits into %s collection",
            tgc_ec_a2_collection
        )

        x_bins_tgc_a2 = []
        y_bins_tgc_a2 = []
        for r in r_bins_tgc:
            for phi in phi_bins_tgc:
                x_bins_tgc_a2.append(r * 10.0 * np.cos(phi))
                y_bins_tgc_a2.append(r * 10.0 * np.sin(phi))  # CORRIGIDO!
        x_bins_tgc_a2 = sorted(list(set(x_bins_tgc_a2)))
        y_bins_tgc_a2 = sorted(list(set(y_bins_tgc_a2)))

        histogramPath = self.HistogramPath + '/TGC_EndcapA_Outer'
        tgc_ec_a2 = MuonHitMaker(
            "MuonHitMaker_TGC_EndcapA_Outer",
            OutputCollectionKey=tgc_ec_a2_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_tgc_a2,
            YBins=y_bins_tgc_a2,
            ZBins=np.linspace(14250.0, 14550.0, 11).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=14250.0,   # mm
            ZMax=14550.0,   # mm
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
        # MDT ENDCAP A - OUTER (Z=1500-1600 cm)
        # -------------------------------------------------
        mdt_ec_a3_collection = "MDT_EndcapA_Outer_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapA Outer and dump hits into %s collection",
            mdt_ec_a3_collection
        )

        x_bins_ec_a3 = []
        y_bins_ec_a3 = []
        for r in r_bins_ec:
            for phi in phi_bins_ec:
                x_bins_ec_a3.append(r * 10.0 * np.cos(phi))
                y_bins_ec_a3.append(r * 10.0 * np.sin(phi))  # CORRIGIDO!
        x_bins_ec_a3 = sorted(list(set(x_bins_ec_a3)))
        y_bins_ec_a3 = sorted(list(set(y_bins_ec_a3)))

        histogramPath = self.HistogramPath + '/MDT_EndcapA_Outer'
        mdt_ec_a3 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapA_Outer",
            OutputCollectionKey=mdt_ec_a3_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec_a3,
            YBins=y_bins_ec_a3,
            ZBins=np.linspace(15000.0, 16000.0, 41).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=15000.0,   # mm
            ZMax=16000.0,   # mm
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
        # MDT ENDCAP C - INNER (Z=-1250 a -1150 cm)
        # -------------------------------------------------
        mdt_ec_c1_collection = "MDT_EndcapC_Inner_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapC Inner and dump hits into %s collection",
            mdt_ec_c1_collection
        )

        x_bins_ec_c1 = []
        y_bins_ec_c1 = []
        for r in r_bins_ec:
            for phi in phi_bins_ec:
                x_bins_ec_c1.append(r * 10.0 * np.cos(phi))
                y_bins_ec_c1.append(r * 10.0 * np.sin(phi))  # CORRIGIDO!
        x_bins_ec_c1 = sorted(list(set(x_bins_ec_c1)))
        y_bins_ec_c1 = sorted(list(set(y_bins_ec_c1)))

        histogramPath = self.HistogramPath + '/MDT_EndcapC_Inner'
        mdt_ec_c1 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapC_Inner",
            OutputCollectionKey=mdt_ec_c1_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec_c1,
            YBins=y_bins_ec_c1,
            ZBins=np.linspace(-12500.0, -11500.0, 21).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=-12500.0,  # mm
            ZMax=-11500.0,  # mm
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
        # TGC ENDCAP C - INNER (Z=-1295 a -1265 cm)
        # -------------------------------------------------
        tgc_ec_c1_collection = "TGC_EndcapC_Inner_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for TGC EndcapC Inner and dump hits into %s collection",
            tgc_ec_c1_collection
        )

        x_bins_tgc_c1 = []
        y_bins_tgc_c1 = []
        for r in r_bins_tgc:
            for phi in phi_bins_tgc:
                x_bins_tgc_c1.append(r * 10.0 * np.cos(phi))
                y_bins_tgc_c1.append(r * 10.0 * np.sin(phi))  # CORRIGIDO!
        x_bins_tgc_c1 = sorted(list(set(x_bins_tgc_c1)))
        y_bins_tgc_c1 = sorted(list(set(y_bins_tgc_c1)))

        histogramPath = self.HistogramPath + '/TGC_EndcapC_Inner'
        tgc_ec_c1 = MuonHitMaker(
            "MuonHitMaker_TGC_EndcapC_Inner",
            OutputCollectionKey=tgc_ec_c1_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_tgc_c1,
            YBins=y_bins_tgc_c1,
            ZBins=np.linspace(-12950.0, -12650.0, 11).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=-12950.0,  # mm
            ZMax=-12650.0,  # mm
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
        # MDT ENDCAP C - MIDDLE (Z=-1380 a -1340 cm)
        # -------------------------------------------------
        mdt_ec_c2_collection = "MDT_EndcapC_Middle_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapC Middle and dump hits into %s collection",
            mdt_ec_c2_collection
        )

        x_bins_ec_c2 = []
        y_bins_ec_c2 = []
        for r in r_bins_ec:
            for phi in phi_bins_ec:
                x_bins_ec_c2.append(r * 10.0 * np.cos(phi))
                y_bins_ec_c2.append(r * 10.0 * np.sin(phi))  # CORRIGIDO!
        x_bins_ec_c2 = sorted(list(set(x_bins_ec_c2)))
        y_bins_ec_c2 = sorted(list(set(y_bins_ec_c2)))

        histogramPath = self.HistogramPath + '/MDT_EndcapC_Middle'
        mdt_ec_c2 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapC_Middle",
            OutputCollectionKey=mdt_ec_c2_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec_c2,
            YBins=y_bins_ec_c2,
            ZBins=np.linspace(-13800.0, -13400.0, 21).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=-13800.0,  # mm
            ZMax=-13400.0,  # mm
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
        # TGC ENDCAP C - OUTER (Z=-1455 a -1425 cm)
        # -------------------------------------------------
        tgc_ec_c2_collection = "TGC_EndcapC_Outer_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for TGC EndcapC Outer and dump hits into %s collection",
            tgc_ec_c2_collection
        )

        x_bins_tgc_c2 = []
        y_bins_tgc_c2 = []
        for r in r_bins_tgc:
            for phi in phi_bins_tgc:
                x_bins_tgc_c2.append(r * 10.0 * np.cos(phi))
                y_bins_tgc_c2.append(r * 10.0 * np.sin(phi))  # CORRIGIDO!
        x_bins_tgc_c2 = sorted(list(set(x_bins_tgc_c2)))
        y_bins_tgc_c2 = sorted(list(set(y_bins_tgc_c2)))

        histogramPath = self.HistogramPath + '/TGC_EndcapC_Outer'
        tgc_ec_c2 = MuonHitMaker(
            "MuonHitMaker_TGC_EndcapC_Outer",
            OutputCollectionKey=tgc_ec_c2_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_tgc_c2,
            YBins=y_bins_tgc_c2,
            ZBins=np.linspace(-14550.0, -14250.0, 11).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=-14550.0,  # mm
            ZMax=-14250.0,  # mm
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
        # MDT ENDCAP C - OUTER (Z=-1600 a -1500 cm)
        # -------------------------------------------------
        mdt_ec_c3_collection = "MDT_EndcapC_Outer_Hits"
        MSG_INFO(
            self,
            "Create new MuonHitMaker for MDT EndcapC Outer and dump hits into %s collection",
            mdt_ec_c3_collection
        )

        x_bins_ec_c3 = []
        y_bins_ec_c3 = []
        for r in r_bins_ec:
            for phi in phi_bins_ec:
                x_bins_ec_c3.append(r * 10.0 * np.cos(phi))
                y_bins_ec_c3.append(r * 10.0 * np.sin(phi))  # CORRIGIDO!
        x_bins_ec_c3 = sorted(list(set(x_bins_ec_c3)))
        y_bins_ec_c3 = sorted(list(set(y_bins_ec_c3)))

        histogramPath = self.HistogramPath + '/MDT_EndcapC_Outer'
        mdt_ec_c3 = MuonHitMaker(
            "MuonHitMaker_MDT_EndcapC_Outer",
            OutputCollectionKey=mdt_ec_c3_collection,
            OutputLevel=self.OutputLevel,
            HistogramPath=histogramPath,
            SamplingNoiseStd=0.0,
            UseCartesianGrid=True,
            DetailedHistograms=True,
            XBins=x_bins_ec_c3,
            YBins=y_bins_ec_c3,
            ZBins=np.linspace(-16000.0, -15000.0, 41).tolist(),  # mm
            RMin=2000.0,    # mm
            RMax=8500.0,    # mm
            ZMin=-16000.0,  # mm
            ZMax=-15000.0,  # mm
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

        # Cria o algoritmo de merge final, tentando importar MuonHitMerge.
        # Se não estiver disponível, apenas avisa e continua sem merge.
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
