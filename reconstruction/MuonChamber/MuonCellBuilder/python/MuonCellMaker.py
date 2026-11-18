# ============================================================================
# Arquivo: MuonCellMaker.py
# Descrição: Wrapper Python para o algoritmo C++ "MuonCellMaker" (ROOT).
#            Configura os parâmetros geométricos e de amostragem necessários
#            para construir as células de muon: bins de eta/phi, faixa de
#            raio (R/ RMin/RMax), detector, sampling, região, layer, range
#            de Bunch Crossing IDs e duração do bunch. Também permite
#            acoplar um gerador de pulso (PulseGenerator) e ferramentas
#            (Tools, e.g. filtro ótimo).
# ============================================================================

__all__ = ["MuonCellMaker"]

# --- Imports do framework e ROOT -------------------------------------------
from GaugiKernel        import Cpp, LoggingLevel
from GaugiKernel.macros import *
import ROOT
import math


class MuonCellMaker(Cpp):
    """
    Encapsula o algoritmo C++ MuonCellMaker, configurando-o via setProperty
    a partir dos parâmetros Python e dos kwargs informados.
    """

    def __init__(self, name,
                 InputHitsKey: str = "MuonHits",
                 OutputCollectionKey: str = "MuonChamberCells",
                 OutputLevel: int = LoggingLevel.toC('INFO'),
                 DetailedHistograms: bool = False,
                 HistogramPath: str = "/Hists/MuonChamberCells",
                 **kwargs):

        # Instancia o objeto C++ subjacente
        Cpp.__init__(self, ROOT.MuonCellMaker(name))

        # Ferramentas acopláveis (ex.: filtro ótimo) e gerador de pulso
        self.Tools = []
        self.PulseGenerator = None

        # --- Informações vindas dos kwargs ------------------------------
        detector_type = kwargs.get('detector_type', 'MDT')
        sampling      = kwargs.get('sampling', 1)
        region        = kwargs.get('region', 0)
        layer         = kwargs.get('layer', 0)

        # Converte o nome do detector para o código inteiro esperado pelo C++
        detector_map = {'MDT': 0, 'RPC': 1, 'TGC': 2, 'CSC': 3}
        detector = detector_map.get(detector_type, 0)

        # Range fixo de BCIDs e duração do bunch (25 ns)
        BUNCH_ID_START = -20
        BUNCH_ID_END   = 20
        BUNCH_DURATION = 25

        # --- Log de criação ---------------------------------------------
        print(f"\n{'='*20}")
        print(f"MuonCellMaker: {name}")
        print(f"{'='*20}")
        print(f"  Detector: {detector_type} (code={detector})")
        print(f"  Sampling: {sampling}")
        print(f"  Region: {region}")

        # Determina a localização geométrica a partir do código de região
        if region == 0:
            detector_location = "Barrel"
        elif region == 1:
            detector_location = "EndcapA"
        elif region == 2:
            detector_location = "EndcapC"
        else:
            detector_location = f"Region{region}"

        # --- Configuração geométrica baseada no SAMPLING ----------------
        config = self._get_detector_config(detector_type, sampling, region)

        # --- Construção dos arrays de bordas de eta e phi ---------------
        eta_bins = []
        phi_bins = []

        if 'eta_min' in config and 'eta_max' in config and 'eta_nbins' in config:
            eta_step = (config['eta_max'] - config['eta_min']) / config['eta_nbins']
            eta_bins = [config['eta_min'] + i * eta_step for i in range(config['eta_nbins'] + 1)]
        else:
            eta_bins = [-2.5, 2.5]

        if 'phi_min' in config and 'phi_max' in config and 'phi_nbins' in config:
            phi_step = (config['phi_max'] - config['phi_min']) / config['phi_nbins']
            phi_bins = [config['phi_min'] + i * phi_step for i in range(config['phi_nbins'] + 1)]
        else:
            phi_bins = [-math.pi, math.pi]

        # --- Cálculo dos tamanhos das células (apenas para log) ---------
        eta_cell_size     = (eta_bins[-1] - eta_bins[0]) / config['eta_nbins']
        phi_cell_size     = (phi_bins[-1] - phi_bins[0]) / config['phi_nbins']
        phi_cell_size_deg = phi_cell_size * 180 / math.pi

        print(f"\n  Configuração final:")
        print(f"    BCID Range: [{BUNCH_ID_START}, {BUNCH_ID_END}] = {BUNCH_ID_END - BUNCH_ID_START + 1} samples")
        print(f"    Bunch Duration: {BUNCH_DURATION} ns")
        print(f"    Geometry: {detector_type} {detector_location} (Sampling={sampling})")
        print(f"    Eta: {config['eta_nbins']} bins, range [{eta_bins[0]:.3f}, {eta_bins[-1]:.3f}]")
        print(f"    Phi: {config['phi_nbins']} bins, range [{phi_bins[0]:.3f}, {phi_bins[-1]:.3f}]")
        print(f"    Cell Size: Δη={eta_cell_size:.4f}, Δφ={phi_cell_size:.4f} rad ({phi_cell_size_deg:.2f}°)")
        print(f"    R: {config['r_value']/1000:.1f}m")
        print(f"    Total cells: {config['eta_nbins'] * config['phi_nbins']}")
        print(f"{'='*20}\n")

        # --- Configuração das propriedades do algoritmo C++ -------------
        # IMPORTANTE: os nomes abaixo precisam corresponder exatamente aos
        # nomes esperados pelo C++.
        self.setProperty("InputHitsKey",         InputHitsKey)
        self.setProperty("OutputCollectionKey",  OutputCollectionKey)
        self.setProperty("EtaBins",              eta_bins)
        self.setProperty("PhiBins",              phi_bins)
        self.setProperty("RMin",                 config['r_min'])
        self.setProperty("RMax",                 config['r_max'])
        self.setProperty("R",                    config['r_value'])
        self.setProperty("Layer",                layer)
        self.setProperty("Sampling",             sampling)
        self.setProperty("Detector",             detector)     
        self.setProperty("Region",               region)
        self.setProperty("BunchIdStart",         BUNCH_ID_START)
        self.setProperty("BunchIdEnd",           BUNCH_ID_END)
        self.setProperty("BunchDuration",        BUNCH_DURATION)
        self.setProperty("DetailedHistograms",   DetailedHistograms)
        self.setProperty("HistogramPath",        HistogramPath)
        self.setProperty("OutputLevel",          OutputLevel)

    def _get_detector_config(self, detector_type, sampling, region):
        """Retorna o dicionário de configuração geométrica (raio, faixa
        radial, número e limites de bins em eta/phi) com base no tipo de
        detector, sampling e região."""

        # Configuração default (usada como fallback)
        config = {
            'r_value':   5000.0,
            'r_min':     0.0,
            'r_max':     0.0,
            'eta_nbins': 64,
            'phi_nbins': 64,
            'eta_min':   -2.5,
            'eta_max':   2.5,
            'phi_min':   -math.pi,
            'phi_max':   math.pi,
        }

        # --- MDT (Sampling: 1=Inner, 2=Middle, 3=Outer) -----------------
        if detector_type == 'MDT':
            if region == 0:  # Barrel
                config['phi_nbins'] = 64
                if sampling == 1:
                    config['eta_min'] = -2.0
                    config['eta_max'] = 2.0
                    config['eta_nbins'] = 64
                    config['r_value'] = 5000.0
                    config['r_min']   = 4900.0
                    config['r_max']   = 5100.0
                elif sampling == 2:
                    config['eta_min'] = -2.0
                    config['eta_max'] = 2.0
                    config['eta_nbins'] = 64
                    config['r_value'] = 7500.0
                    config['r_min']   = 7400.0
                    config['r_max']   = 7600.0
                elif sampling == 3:
                    config['eta_min'] = -2.0
                    config['eta_max'] = 2.0
                    config['eta_nbins'] = 64
                    config['r_value'] = 10000.0
                    config['r_min']   = 9900.0
                    config['r_max']   = 10100.0
            else:  # Endcaps
                config['phi_nbins'] = 64
                if region == 1:
                    eta_min = 1.05
                    eta_max = 2.4
                else:
                    eta_min = -2.4
                    eta_max = -1.05
                if sampling in [1, 2, 3]:
                    config['eta_min'] = eta_min
                    config['eta_max'] = eta_max
                    config['eta_nbins'] = 64
                    config['r_value'] = 5500.0 + (sampling - 1) * 1000.0
                    config['r_min']   = config['r_value'] - 500.0
                    config['r_max']   = config['r_value'] + 500.0

        # --- RPC (Sampling: 4=MiddleTrigger, 5=OuterTrigger) ------------
        elif detector_type == 'RPC' and region == 0:
            config['eta_min'] = -2.0
            config['eta_max'] = 2.0
            config['eta_nbins'] = 48
            config['phi_nbins'] = 48
            if sampling == 4:
                config['r_value'] = 7700.0
                config['r_min']   = 7650.0
                config['r_max']   = 7750.0
            elif sampling == 5:
                config['r_value'] = 10300.0
                config['r_min']   = 10250.0
                config['r_max']   = 10350.0

        # --- TGC (Sampling: 6=InnerTrigger, 7=OuterTrigger) -------------
        elif detector_type == 'TGC' and region != 0:
            config['phi_nbins'] = 48
            if region == 1:
                eta_min = 1.05
                eta_max = 2.4
            else:
                eta_min = -2.4
                eta_max = -1.05
            if sampling == 6:
                config['eta_min'] = eta_min
                config['eta_max'] = eta_max
                config['eta_nbins'] = 64
                config['r_value'] = 5000.0
                config['r_min']   = 4500.0
                config['r_max']   = 5500.0
            elif sampling == 7:
                config['eta_min'] = eta_min
                config['eta_max'] = eta_max
                config['eta_nbins'] = 48
                config['r_value'] = 6500.0
                config['r_min']   = 6000.0
                config['r_max']   = 7000.0

        return config

    def core(self):
        """Monta o objeto C++ final, anexando as ferramentas e o gerador
        de pulso ao core antes de retorná-lo."""
        # Anexa todas as Tools (ex.: filtro ótimo) ao core C++
        for tool in self.Tools:
            self._core.push_back(tool.core())

        # Acopla o gerador de pulso, se houver
        if self.PulseGenerator:
            self._core.setPulseGenerator(self.PulseGenerator.core())

        return self._core

    def __add__(self, tool):
        """Permite usar o operador '+' para acoplar ferramentas (Tools)."""
        self.Tools.append(tool)
        return self
