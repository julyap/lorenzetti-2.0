# ============================================================================
# Arquivo: MuonHitMaker.py
# Descrição: Wrapper Python para o algoritmo C++ "MuonHitMaker". Encapsula
#            a configuração do detector e da geometria (faixa em R/Z, grade
#            cartesiana em X/Y/Z, sampling, região, duração do bunch, etc.)
#            e converte o nome do detector (string) para o código inteiro
#            esperado pelo C++ (0=MDT, 1=RPC, 2=TGC). Também gera
#            automaticamente os grids em X/Y/Z a partir de RMin/RMax e
#            ZMin/ZMax quando eles não são fornecidos explicitamente.
#            Permite acoplar ferramentas (Tools) e é compatível com o
#            operador '+' para encadear ferramentas.
# ============================================================================

__all__ = ["MuonHitMaker"]

# --- Imports do framework e utilitários ------------------------------------
from GaugiKernel        import Cpp, LoggingLevel
from GaugiKernel.macros import *
import ROOT
import numpy as np


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
                  # Geometry parameters
                  XBins                : list   = None,
                  YBins                : list   = None,
                  ZBins                : list   = None,
                  RMin                 : float  = 350.0,
                  RMax                 : float  = 750.0,
                  ZMin                 : float  = -800.0,
                  ZMax                 : float  = 800.0,
                  # Identificação do detector (ALINHADO COM O C++)
                  Detector             : str or int = None,  # Pode ser string ("MDT") ou int (0)
                  Region               : int    = 0,     # Local: 0=Barrel, 1=Endcap+, 2=Endcap-
                  Sampling             : int    = 1,     # Camada: 1=MDT_Inner, 2=MDT_Middle, 3=MDT_Outer, 4=RPC_Inner, 5=RPC_Outer, 6=TGC_Inner, 7=TGC_Outer
                  # Parâmetros de segmentação
                  BunchIdStart         : int    = -20,
                  BunchIdEnd           : int    = 20,
                  BunchDuration        : float  = 25.0,
                  DetailedHistograms   : bool   = False,
                  # Parâmetros auxiliares para configuração automática
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
        # O C++ espera um inteiro como código de detector. Se o usuário
        # passar uma string ("MDT"/"RPC"/"TGC"), convertemos aqui.
        detector_map = {
            "MDT": 0,
            "RPC": 1,
            "TGC": 2
        }

        # Converter Detector para int se for string
        if isinstance(Detector, str):
            if Detector in detector_map:
                detector_int = detector_map[Detector]
                print(f"DEBUG [MuonHitMaker]: Convertendo Detector='{Detector}' para int={detector_int}")
            else:
                detector_int = 0  # Default MDT
                print(f"WARNING [MuonHitMaker]: Detector desconhecido '{Detector}', usando MDT (0)")
        else:
            detector_int = Detector if Detector is not None else 0
        if detector_int == 1:  # RPC
            print(f"🔴 RPC CONFIGURADO: Sampling={Sampling}, Region={Region}, R=[{RMin},{RMax}] cm, Z=[{ZMin},{ZMax}] cm")

        # ============================================================
        # MAPEAMENTO CORRIGIDO PARA DEBUG
        # ============================================================
        detector_names = {0: "MDT", 1: "RPC", 2: "TGC"}
        region_names = {0: "Barrel", 1: "Endcap+", 2: "Endcap-"}

        # NOMENCLATURA CORRIGIDA DOS SAMPLINGS
        layer_names = {
            1: "MDT_Inner",
            2: "MDT_Middle",
            3: "MDT_Outer",
            4: "RPC_Inner",
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
        # CONFIGURAÇÃO DA GEOMETRIA POR DETECTOR/REGIÃO/CAMADA
        # COM RAIOS CORRETOS PARA CADA CAMADA NO BARREL
        # ============================================================

        # BARREL (Region=0)
        if Region == 0:
            if detector_int == 0:  # MDT
                if Sampling == 1:  # MDT_Inner
                    RMin, RMax = 350.0, 400.0
                    ZMin, ZMax = -800.0, 800.0
                    print(f"DEBUG [MuonHitMaker]: MDT Barrel Inner: R=[350,400] cm, Z=[-800,800] cm")
                elif Sampling == 2:  # MDT_Middle
                    RMin, RMax = 500.0, 550.0
                    ZMin, ZMax = -800.0, 800.0
                    print(f"DEBUG [MuonHitMaker]: MDT Barrel Middle: R=[500,550] cm, Z=[-800,800] cm")
                elif Sampling == 3:  # MDT_Outer
                    RMin, RMax = 700.0, 750.0
                    ZMin, ZMax = -800.0, 800.0
                    print(f"DEBUG [MuonHitMaker]: MDT Barrel Outer: R=[700,750] cm, Z=[-800,800] cm")
                else:
                    print(f"WARNING [MuonHitMaker]: MDT Barrel com Sampling={Sampling} inválido! Usando MDT_Inner como default")
                    RMin, RMax = 350.0, 400.0
                    ZMin, ZMax = -800.0, 800.0

            elif detector_int == 1:  # RPC
                if Sampling == 4:  # RPC_Inner
                    RMin, RMax = 420.0, 450.0
                    ZMin, ZMax = -800.0, 800.0
                    print(f"DEBUG [MuonHitMaker]: RPC Barrel Inner: R=[420,450] cm, Z=[-800,800] cm")
                elif Sampling == 5:  # RPC_Outer
                    RMin, RMax = 600.0, 650.0
                    ZMin, ZMax = -800.0, 800.0
                    print(f"DEBUG [MuonHitMaker]: RPC Barrel Outer: R=[600,650] cm, Z=[-800,800] cm")
                else:
                    print(f"WARNING [MuonHitMaker]: RPC Barrel com Sampling={Sampling} inválido! Usando RPC_Inner como default")
                    RMin, RMax = 420.0, 450.0
                    ZMin, ZMax = -800.0, 800.0

            elif detector_int == 2:  # TGC
                print(f"WARNING [MuonHitMaker]: TGC não existe no Barrel! Apenas Endcaps.")
                # Fallback para valores padrão
                RMin, RMax = 350.0, 750.0
                ZMin, ZMax = -800.0, 800.0

        # ENDCAP POSITIVO (Region=1)
        elif Region == 1:
            if detector_int == 0:  # MDT
                if Sampling == 1:  # MDT_Inner
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = 1150.0, 1250.0
                    print(f"DEBUG [MuonHitMaker]: MDT Endcap+ Inner: Z=[1150,1250] cm, R=[200,850] cm")
                elif Sampling == 2:  # MDT_Middle
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = 1340.0, 1380.0
                    print(f"DEBUG [MuonHitMaker]: MDT Endcap+ Middle: Z=[1340,1380] cm, R=[200,850] cm")
                elif Sampling == 3:  # MDT_Outer
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = 1500.0, 1540.0
                    print(f"DEBUG [MuonHitMaker]: MDT Endcap+ Outer: Z=[1500,1540] cm, R=[200,850] cm")
                else:
                    print(f"WARNING [MuonHitMaker]: MDT Endcap+ com Sampling={Sampling} inválido! Usando MDT_Inner como default")
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = 1150.0, 1250.0

            elif detector_int == 1:  # RPC
                print(f"WARNING [MuonHitMaker]: RPC não existe no Endcap! Apenas Barrel.")
                # Fallback para valores padrão
                RMin, RMax = 200.0, 850.0
                ZMin, ZMax = 1150.0, 1250.0

            elif detector_int == 2:  # TGC
                if Sampling == 6:  # TGC_Inner
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = 1265.0, 1295.0
                    print(f"DEBUG [MuonHitMaker]: TGC Endcap+ Inner: Z=[1265,1295] cm, R=[200,850] cm")
                elif Sampling == 7:  # TGC_Outer
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = 1425.0, 1455.0
                    print(f"DEBUG [MuonHitMaker]: TGC Endcap+ Outer: Z=[1425,1455] cm, R=[200,850] cm")
                else:
                    print(f"WARNING [MuonHitMaker]: TGC Endcap+ com Sampling={Sampling} inválido! Usando TGC_Inner como default")
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = 1265.0, 1295.0

        # ENDCAP NEGATIVO (Region=2)
        elif Region == 2:
            if detector_int == 0:  # MDT
                if Sampling == 1:  # MDT_Inner
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = -1250.0, -1150.0
                    print(f"DEBUG [MuonHitMaker]: MDT Endcap- Inner: Z=[-1250,-1150] cm, R=[200,850] cm")
                elif Sampling == 2:  # MDT_Middle
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = -1380.0, -1340.0
                    print(f"DEBUG [MuonHitMaker]: MDT Endcap- Middle: Z=[-1380,-1340] cm, R=[200,850] cm")
                elif Sampling == 3:  # MDT_Outer
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = -1540.0, -1500.0
                    print(f"DEBUG [MuonHitMaker]: MDT Endcap- Outer: Z=[-1540,-1500] cm, R=[200,850] cm")
                else:
                    print(f"WARNING [MuonHitMaker]: MDT Endcap- com Sampling={Sampling} inválido! Usando MDT_Inner como default")
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = -1250.0, -1150.0

            elif detector_int == 1:  # RPC
                print(f"WARNING [MuonHitMaker]: RPC não existe no Endcap! Apenas Barrel.")
                # Fallback para valores padrão
                RMin, RMax = 200.0, 850.0
                ZMin, ZMax = -1250.0, -1150.0

            elif detector_int == 2:  # TGC
                if Sampling == 6:  # TGC_Inner
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = -1295.0, -1265.0
                    print(f"DEBUG [MuonHitMaker]: TGC Endcap- Inner: Z=[-1295,-1265] cm, R=[200,850] cm")
                elif Sampling == 7:  # TGC_Outer
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = -1455.0, -1425.0
                    print(f"DEBUG [MuonHitMaker]: TGC Endcap- Outer: Z=[-1455,-1425] cm, R=[200,850] cm")
                else:
                    print(f"WARNING [MuonHitMaker]: TGC Endcap- com Sampling={Sampling} inválido! Usando TGC_Inner como default")
                    RMin, RMax = 200.0, 850.0
                    ZMin, ZMax = -1295.0, -1265.0

        # ============================================================
        # CRIAR GRID EM X,Y,Z BASEADO NOS PARÂMETROS
        # ============================================================

        # Para o barrel (Region=0), grid cilíndrico
        if Region == 0:  # Barrel
            # Criar bins em phi
            phi_min = -np.pi
            phi_max = np.pi
            phi_bins = np.linspace(phi_min, phi_max, NPhiSegments + 1)

            # IMPORTANTE: Para cada detector/sampling, usar seu raio específico!
            # O raio médio é calculado baseado nos RMin e RMax configurados acima
            avg_radius = (RMin + RMax) / 2.0

            # CRIAÇÃO CORRETA DOS BINS EM X E Y PARA O BARREL
            # x = r * cos(phi), y = r * sin(phi)
            x_bins = []
            y_bins = []
            for phi in phi_bins:
                x_bins.append(avg_radius * np.cos(phi))
                y_bins.append(avg_radius * np.sin(phi))  # <-- CORRIGIDO: sin(phi) em vez de cos(phi)

            # Garantir que os bins estão ordenados
            x_bins = sorted(x_bins)
            y_bins = sorted(y_bins)

            XBins = x_bins
            YBins = y_bins

            # Criar bins em Z para o barrel
            z_bins = np.linspace(ZMin, ZMax, 33).tolist()
            ZBins = z_bins

        else:  # Endcaps (Region=1 ou 2)
            # Para endcaps, grid em X,Y circular
            r_bins = np.linspace(RMin, RMax, 9).tolist()
            phi_bins = np.linspace(-np.pi, np.pi, 17).tolist()

            # Gerar pontos no círculo
            x_points = []
            y_points = []
            for r in r_bins:
                for phi in phi_bins:
                    x_points.append(r * np.cos(phi))
                    y_points.append(r * np.sin(phi))  # <-- CORRIGIDO

            # Criar bins únicos ordenados
            x_bins = sorted(list(set([round(x, 1) for x in x_points])))
            y_bins = sorted(list(set([round(y, 1) for y in y_points])))

            XBins = x_bins
            YBins = y_bins

            # Para endcaps, bins em Z
            z_bins = np.linspace(ZMin, ZMax, 9).tolist()
            ZBins = z_bins

        # Fallback para defaults se algo deu errado
        if XBins is None or len(XBins) < 2:
            XBins = [-500.0, -400.0, -300.0, -200.0, -100.0, 0.0, 100.0, 200.0, 300.0, 400.0, 500.0]
        if YBins is None or len(YBins) < 2:
            YBins = [-500.0, -400.0, -300.0, -200.0, -100.0, 0.0, 100.0, 200.0, 300.0, 400.0, 500.0]
        if ZBins is None or len(ZBins) < 2:
            ZBins = [-700.0, -600.0, -500.0, -400.0, -300.0, -200.0, -100.0, 0.0, 100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0]

        # ============================================================
        # SET PROPERTIES - ALINHADO COM O C++
        # ============================================================
        self.setProperty( "OutputCollectionKey" , OutputCollectionKey )   # chave da coleção de saída
        self.setProperty( "OutputLevel"         , OutputLevel         )   # nível de log
        self.setProperty( "HistogramPath"       , HistogramPath       )   # caminho dos histogramas
        self.setProperty( "SamplingNoiseStd"    , SamplingNoiseStd    )   # ruído na amostragem
        self.setProperty( "UseCartesianGrid"    , UseCartesianGrid    )   # usa grade cartesiana?

        self.setProperty( "XBins"               , XBins               )   # bins em X
        self.setProperty( "YBins"               , YBins               )   # bins em Y
        self.setProperty( "ZBins"               , ZBins               )   # bins em Z
        self.setProperty( "RMin"                , RMin                )   # raio mínimo
        self.setProperty( "RMax"                , RMax                )   # raio máximo
        self.setProperty( "ZMin"                , ZMin                )   # Z mínimo
        self.setProperty( "ZMax"                , ZMax                )   # Z máximo

        # PROPRIEDADES PRINCIPAIS - DETECTOR AGORA É INT
        self.setProperty( "Detector"             , detector_int        )  # INT: 0=MDT, 1=RPC, 2=TGC
        self.setProperty( "Region"               , Region              )  # Local: Barrel, Endcap
        self.setProperty( "Sampling"             , Sampling            )  # Camada: 1=MDT_Inner, 2=MDT_Middle, 3=MDT_Outer, 4=RPC_Inner, 5=RPC_Outer, 6=TGC_Inner, 7=TGC_Outer

        self.setProperty( "BunchIdStart"        , BunchIdStart        )   # BCID inicial
        self.setProperty( "BunchIdEnd"          , BunchIdEnd          )   # BCID final
        self.setProperty( "BunchDuration"       , BunchDuration       )   # duração do bunch (ns)
        self.setProperty( "DetailedHistograms"  , DetailedHistograms  )   # histogramas detalhados?

        # DEBUG FINAL
        print(f"DEBUG [MuonHitMaker]: Configuração final:")
        print(f"DEBUG [MuonHitMaker]:   Detector={detector_int} ({detector_name})")
        print(f"DEBUG [MuonHitMaker]:   Region={Region} ({region_name})")
        print(f"DEBUG [MuonHitMaker]:   Sampling={Sampling} ({layer_name})")
        print(f"DEBUG [MuonHitMaker]:   CollectionKey={OutputCollectionKey}")
        print(f"DEBUG [MuonHitMaker]:   R range: [{RMin}, {RMax}] cm")
        print(f"DEBUG [MuonHitMaker]:   Z range: [{ZMin}, {ZMax}] cm")
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
