# ============================================================================
# Arquivo: MuonHitMaker.py
# Descrição: Wrapper Python para o algoritmo C++ "MuonHitMaker". Encapsula
#            a configuração do detector e da geometria (faixa em R/Z, grade
#            cartesiana em X/Y/Z, sampling, região, duração do bunch, etc.)
#            e converte o nome do detector (string) para o código inteiro
#            esperado pelo C++ (0=MDT, 1=RPC, 2=TGC).
#
#            IMPORTANTE:
#              - Este arquivo NÃO sobrescreve RMin/RMax/ZMin/ZMax. Todos os
#                valores vêm do MuonHitBuilder (em mm), que por sua vez está
#                alinhado com a geometria em ATLAS/MuonChamber.py e
#                ATLAS/MuonEndcap.py.
#              - Os bins em X/Y são gerados com _make_bins(), que remove
#                duplicatas, ordena e garante bins ESTRITAMENTE CRESCENTES,
#                evitando o erro do ROOT:
#                  "TAxis::Set: bins must be in increasing order"
# ============================================================================

__all__ = ["MuonHitMaker"]

# --- Imports do framework e utilitários ------------------------------------
from GaugiKernel        import Cpp, LoggingLevel
from GaugiKernel.macros import *
import ROOT
import numpy as np


# ---------------------------------------------------------------------------
# Helper: bins estritamente crescentes e sem duplicatas
# ---------------------------------------------------------------------------
def _make_bins(values, decimals=2):
    """
    Recebe uma lista de valores (possivelmente com duplicatas e fora de
    ordem), arredonda para 'decimals' casas, remove duplicatas, ordena e
    garante que cada bin seja estritamente maior que o anterior.

    Isso evita o erro do ROOT:
        Error in <TAxis::TAxis::Set>: bins must be in increasing order

    Retorna uma lista Python de floats.
    """
    if values is None:
        return None
    s = sorted(set(round(float(v), decimals) for v in values))
    out = []
    for v in s:
        if not out or v > out[-1]:
            out.append(v)
    return out


class MuonHitMaker( Cpp ):
    """
    Encapsula o algoritmo C++ MuonHitMaker, configurando-o a partir dos
    parâmetros Python (geometria, detector, região, sampling, etc.).
    """

    def __init__( self, name,
                  OutputCollectionKey  : str    = None,
                  OutputLevel          : int    = LoggingLevel.toC('INFO'),
                  HistogramPath        : str    = "/MuonHitMaker",
                  SamplingNoiseStd     : float  = 0.0,
                  UseCartesianGrid     : bool   = True,
                  # Geometry parameters (R e Z em mm)
                  XBins                : list   = None,
                  YBins                : list   = None,
                  ZBins                : list   = None,
                  RMin                 : float  = 5000.0,
                  RMax                 : float  = 5500.0,
                  ZMin                 : float  = -3500.0,
                  ZMax                 : float  = +3500.0,
                  # Identificação do detector (ALINHADO COM O C++)
                  Detector             : str or int = None,  # "MDT"/"RPC"/"TGC" ou 0/1/2
                  Region               : int    = 0,     # 0=Barrel, 1=Endcap+, 2=Endcap-
                  Sampling             : int    = 1,     # 1..7 (ver layer_names)
                  # Parâmetros de segmentação
                  BunchIdStart         : int    = -20,
                  BunchIdEnd           : int    = 20,
                  BunchDuration        : float  = 25.0,
                  DetailedHistograms   : bool   = False,
                  # Parâmetros auxiliares
                  Station              : int    = None,
                  NPhiSegments         : int    = 16,
                  NEtaSegments         : int    = 1,
                  ):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonHitMaker(name) )

        # Lista de ferramentas acopláveis
        self.Tools = []

        # ============================================================
        # CONVERTER DETECTOR STRING PARA INT
        # ============================================================
        detector_map = {
            "MDT": 0,
            "RPC": 1,
            "TGC": 2
        }

        if isinstance(Detector, str):
            if Detector in detector_map:
                detector_int = detector_map[Detector]
                print(f"DEBUG [MuonHitMaker]: Convertendo Detector='{Detector}' para int={detector_int}")
            else:
                detector_int = 0
                print(f"WARNING [MuonHitMaker]: Detector desconhecido '{Detector}', usando MDT (0)")
        else:
            detector_int = Detector if Detector is not None else 0

        if detector_int == 1:  # RPC
            print(f"🔴 RPC CONFIGURADO: Sampling={Sampling}, Region={Region}, "
                  f"R=[{RMin},{RMax}] mm, Z=[{ZMin},{ZMax}] mm")

        # ============================================================
        # MAPEAMENTO PARA DEBUG
        # ============================================================
        detector_names = {0: "MDT", 1: "RPC", 2: "TGC"}
        region_names   = {0: "Barrel", 1: "Endcap+", 2: "Endcap-"}

        layer_names = {
            1: "MDT_Inner",
            2: "MDT_Middle",
            3: "MDT_Outer",
            4: "RPC_Middle",
            5: "RPC_Outer",
            6: "TGC_Inner",
            7: "TGC_Outer"
        }

        detector_name = detector_names.get(detector_int, f"Detector{detector_int}")
        region_name   = region_names.get(Region, f"Region{Region}")
        layer_name    = layer_names.get(Sampling, f"Layer{Sampling}")

        # ============================================================
        # GERAR COLLECTIONKEY ÚNICA SE NÃO FORNECIDA
        # ============================================================
        if OutputCollectionKey is None:
            OutputCollectionKey = f"{detector_name}_{layer_name}_{region_name}_Hits"
            print(f"DEBUG [MuonHitMaker]: CollectionKey gerada: {OutputCollectionKey}")

        # ============================================================
        # GRID EM X,Y,Z
        #
        # Não sobrescrevemos RMin/RMax/ZMin/ZMax: eles vêm do builder, já em
        # mm e alinhados com a geometria. Aqui apenas geramos a grade
        # cartesiana a partir deles, usando _make_bins() para garantir bins
        # estritamente crescentes (evita o erro do TAxis).
        # ============================================================

        # -------------------- BARREL (Region == 0) --------------------
        if Region == 0:
            if XBins is None or YBins is None:
                phi_bins = np.linspace(-np.pi, np.pi, NPhiSegments + 1)
                avg_radius = 0.5 * (RMin + RMax)  # mm

                XBins = _make_bins([avg_radius * np.cos(p) for p in phi_bins])
                YBins = _make_bins([avg_radius * np.sin(p) for p in phi_bins])

            if ZBins is None:
                ZBins = np.linspace(ZMin, ZMax, 33).tolist()

        # -------------------- ENDCAPS (Region == 1 ou 2) --------------
        else:
            if XBins is None or YBins is None:
                n_r   = 9
                n_phi = 17
                r_bins   = np.linspace(RMin, RMax, n_r).tolist()   # mm
                phi_bins = np.linspace(-np.pi, np.pi, n_phi).tolist()

                xs = []
                ys = []
                for r in r_bins:
                    for p in phi_bins:
                        xs.append(r * np.cos(p))
                        ys.append(r * np.sin(p))

                XBins = _make_bins(xs)
                YBins = _make_bins(ys)

            if ZBins is None:
                ZBins = np.linspace(ZMin, ZMax, 9).tolist()

        # -------------------- Sanidade final --------------------------
        # Reaplica _make_bins por segurança (caso o usuário tenha passado
        # XBins/YBins com duplicatas ou fora de ordem).
        if XBins is None or len(XBins) < 2:
            XBins = np.linspace(-RMax, RMax, 21).tolist()
        else:
            XBins = _make_bins(XBins)

        if YBins is None or len(YBins) < 2:
            YBins = np.linspace(-RMax, RMax, 21).tolist()
        else:
            YBins = _make_bins(YBins)

        if ZBins is None or len(ZBins) < 2:
            ZBins = np.linspace(ZMin, ZMax, 33).tolist()
        else:
            # ZBins vindos do builder já são monotônicos, mas garantimos
            ZBins = sorted(set(round(float(z), 2) for z in ZBins))

        # ============================================================
        # SET PROPERTIES - ALINHADO COM O C++
        # ============================================================
        self.setProperty( "OutputCollectionKey" , OutputCollectionKey )
        self.setProperty( "OutputLevel"         , OutputLevel         )
        self.setProperty( "HistogramPath"       , HistogramPath       )
        self.setProperty( "SamplingNoiseStd"    , SamplingNoiseStd    )
        self.setProperty( "UseCartesianGrid"    , UseCartesianGrid    )

        self.setProperty( "XBins"               , XBins               )
        self.setProperty( "YBins"               , YBins               )
        self.setProperty( "ZBins"               , ZBins               )
        self.setProperty( "RMin"                , RMin                )   # mm
        self.setProperty( "RMax"                , RMax                )   # mm
        self.setProperty( "ZMin"                , ZMin                )   # mm
        self.setProperty( "ZMax"                , ZMax                )   # mm

        self.setProperty( "Detector"            , detector_int        )   # INT: 0=MDT, 1=RPC, 2=TGC
        self.setProperty( "Region"              , Region              )   # 0=Barrel, 1=Endcap+, 2=Endcap-
        self.setProperty( "Sampling"            , Sampling            )   # 1..7

        self.setProperty( "BunchIdStart"        , BunchIdStart        )
        self.setProperty( "BunchIdEnd"          , BunchIdEnd          )
        self.setProperty( "BunchDuration"       , BunchDuration       )
        self.setProperty( "DetailedHistograms"  , DetailedHistograms  )

        # DEBUG FINAL
        print(f"DEBUG [MuonHitMaker]: Configuração final:")
        print(f"DEBUG [MuonHitMaker]:   Detector={detector_int} ({detector_name})")
        print(f"DEBUG [MuonHitMaker]:   Region={Region} ({region_name})")
        print(f"DEBUG [MuonHitMaker]:   Sampling={Sampling} ({layer_name})")
        print(f"DEBUG [MuonHitMaker]:   CollectionKey={OutputCollectionKey}")
        print(f"DEBUG [MuonHitMaker]:   R range: [{RMin}, {RMax}] mm")
        print(f"DEBUG [MuonHitMaker]:   Z range: [{ZMin}, {ZMax}] mm")
        print(f"DEBUG [MuonHitMaker]:   Grid: {len(XBins)-1} x-bins, {len(YBins)-1} y-bins, {len(ZBins)-1} z-bins")

    def core(self):
        """Anexa todas as Tools ao core C++ antes de retorná-lo."""
        for tool in self.Tools:
            self._core.push_back(tool.core())
        return self._core

    def __add__( self, tool ):
        """Permite usar o operador '+' para acoplar ferramentas (Tools)."""
        self.Tools += tool
        return self
