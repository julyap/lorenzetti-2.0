# ============================================================================
# Arquivo: MuonCellBuilder.py
# Descrição: Builder responsável por montar toda a cadeia de reconstrução
#            de células de muon. Ele percorre as "samplings" do detector,
#            identifica quais são de muon (MDT/RPC/TGC), extrai as
#            propriedades geométricas de cada uma (tipo de câmara, região,
#            station, layer, bins de eta/phi, raio, ruído, shaper, pesos do
#            filtro ótimo etc.) e cria, para cada sampling, uma instância de
#            MuonCellMaker configurada (com MuonPulseGenerator e
#            MuonOptimalFilter acoplados). Ao final, cria um MuonCellMerge
#            para juntar todas as coleções produzidas em uma única coleção
#            de células (MuonCells) e de células de verdade (TruthMuonCells).
# ============================================================================

__all__ = ["MuonCellBuilder"]

# --- Imports do framework Gaugi/Kernel -------------------------------------
from GaugiKernel        import Logger, LoggingLevel
from GaugiKernel.macros import *
import time

# --- Imports dos componentes internos de reconstrução de muon --------------
from MuonCellMaker        import MuonCellMaker
from MuonCellMerge        import MuonCellMerge
from MuonPulseGenerator   import MuonPulseGenerator
from MuonOptimalFilter    import MuonOptimalFilter
from MuonFlags            import MuonFlags


class MuonCellBuilder(Logger):
    """
    Builder que configura e encadeia os algoritmos de reconstrução de células
    de muon. Herda de Logger para emitir mensagens de log padronizadas.
    """

    def __init__(self, name, detector,
                 HistogramPath="Expert",
                 InputHitsKey="MuonHits",
                 OutputCellsKey="MuonCells",
                 OutputTruthCellsKey="TruthMuonCells",
                 OutputLevel=LoggingLevel.toC('INFO'),
                 doNoise=True,
                 noiseFactor=1.0,
                 doCrossTalk=False,
                 doDetailedHistograms=False,
                 **kwargs):

        # Inicializa o Logger do Gaugi
        Logger.__init__(self)

        # Lista interna que vai acumular todos os algoritmos de reconstrução
        self.__recoAlgs = []

        # Parâmetros de configuração vindos do usuário
        self.HistogramPath          = HistogramPath
        self.OutputLevel            = OutputLevel
        self.InputHitsKey           = InputHitsKey
        self.OutputCellsKey         = OutputCellsKey
        self.OutputTruthCellsKey    = OutputTruthCellsKey
        self.Detector               = detector
        self.OutputCollectionKeys   = []   # chaves das coleções produzidas
        self.MuonCellKeys           = []   # chaves das células de muon geradas
        self.doNoise                = doNoise
        self.noiseFactor            = noiseFactor
        self.doCrossTalk            = doCrossTalk
        self.doDetailedHistograms   = doDetailedHistograms
        self.kwargs                 = kwargs

        # Contadores usados para gerar nomes únicos de algoritmos e coleções
        self.alg_counter        = {}
        self.collection_counter = {}

        # Dicionário com propriedades padrão por tipo de câmara.
        # Serve de fallback quando a sampling não define explicitamente
        # algum parâmetro (shaper, ruído, número de amostras, bins, raio...).
        self.chamber_properties = {
            'MDT': {
                'shaper_file': "/home/administrador/apptainer-1.3.0/lorenzetti/geometry/ATLAS/data/pulseMDT.dat",
                'default_noise': 0.005,
                'default_samples': 10,
                'efficiency': 0.98,
                'eta_bins_barrel': 64,
                'eta_bins_endcap': 64,
                'phi_bins': 64,
                'r_barrel': 5000.0,
                'r_endcap': 5500.0,
            },
            'RPC': {
                'shaper_file': "/home/administrador/apptainer-1.3.0/lorenzetti/geometry/ATLAS/data/pulseRPC.dat",
                'default_noise':  0.0005,
                'default_samples': 3,
                'efficiency': 0.99,
                'eta_bins_barrel': 48,
                'phi_bins': 48,
                'r_barrel': 7700.0,
                'r_endcap': None,
            },
            'TGC': {
                'shaper_file': "/home/administrador/apptainer-1.3.0/lorenzetti/geometry/ATLAS/data/pulseTGC.dat",
                'default_noise':  0.0005,
                'default_samples': 3,
                'efficiency': 0.97,
                'eta_bins_inner': 64,
                'eta_bins_outer': 48,
                'phi_bins': 48,
                'r_inner': 5000.0,
                'r_outer': 6500.0,
            },
        }

    def get_muon_cell_keys(self):
        """Retorna a lista de chaves de células de muon geradas."""
        return self.MuonCellKeys

    def _get_region_name(self, region_code):
        """Converte o código numérico/string da região em um nome legível
        (Barrel, EndcapA, EndcapC)."""
        if region_code == 0:
            return "Barrel"
        elif region_code == 1:
            return "EndcapA"
        elif region_code == 2:
            return "EndcapC"
        elif isinstance(region_code, str):
            if "A" in region_code.upper() or "POS" in region_code.upper():
                return "EndcapA"
            elif "C" in region_code.upper() or "NEG" in region_code.upper():
                return "EndcapC"
            else:
                return "Barrel"
        else:
            return f"Region{region_code}"

    def _get_station_name(self, chamber_type, sampling_id, region_name):
        """Traduz (tipo de câmara, sampling id) para o nome da station
        correspondente (Inner/Middle/Outer/Trigger etc.)."""
        if chamber_type == 'MDT':
            if sampling_id == 1:
                return "Inner"
            elif sampling_id == 2:
                return "Middle"
            elif sampling_id == 3:
                return "Outer"
        elif chamber_type == 'RPC':
            if sampling_id == 4:
                return "MiddleTrigger"
            elif sampling_id == 5:
                return "OuterTrigger"
        elif chamber_type == 'TGC':
            if sampling_id == 5:
                return "InnerTrigger"
            elif sampling_id == 6:
                return "OuterTrigger"
        return f"Station{sampling_id}"

    def _get_unique_alg_name(self, chamber_type, sampling_id, region_name, station_name):
        """Gera um nome único para o algoritmo MuonCellMaker. Se já existir
        um algoritmo com a mesma (câmara, sampling, região), acrescenta um
        sufixo numérico para evitar colisões."""
        base_name = f"MuonCellMaker_{chamber_type}_{station_name}_{region_name}"
        key = f"{chamber_type}_{sampling_id}_{region_name}"
        if key not in self.alg_counter:
            self.alg_counter[key] = 1
            return base_name
        else:
            self.alg_counter[key] += 1
            return f"{base_name}_{self.alg_counter[key]}"

    def _get_unique_collection_name(self, chamber_type, station_name, region_name):
        """Gera um nome único para a coleção de saída de cada sampling."""
        base_name = f"Collection_{chamber_type}_{station_name}_{region_name}"
        key = f"{chamber_type}_{station_name}_{region_name}"
        if key not in self.collection_counter:
            self.collection_counter[key] = 1
            return base_name
        else:
            self.collection_counter[key] += 1
            return f"{base_name}_{self.collection_counter[key]}"

    def _is_valid_muon_sampling(self, samp):
        """Verifica se a sampling pertence a um detector de muon
        (MDT/RPC/TGC/CSC/MUON)."""
        if hasattr(samp, 'Detector'):
            det_str = str(samp.Detector).upper()
            if any(x in det_str for x in ['MDT', 'RPC', 'TGC', 'CSC', 'MUON']):
                return True
        return False

    def _get_sampling_name_safe(self, samp):
        """Obtém o nome da sampling de forma robusta, cobrindo diferentes
        formas de atributo (callable, string, Detector+Sampling, etc.)."""
        try:
            if hasattr(samp, 'name') and callable(samp.name):
                return samp.name()
            elif hasattr(samp, 'name'):
                return str(samp.name)
            elif hasattr(samp, 'Detector'):
                detector = str(samp.Detector)
                sampling = getattr(samp, 'Sampling', '0')
                return f"{detector}_Sampling_{sampling}"
            else:
                return samp.__class__.__name__
        except:
            return "UnknownMuonSampling"

    def _get_eta_bins(self, chamber_type, sampling_id, region):
        """Define os limites dos bins em eta para o tipo de câmara/região.
        Cada elemento é uma borda (n_bins + 1 elementos no total)."""
        if chamber_type == 'MDT':
            if region == 0:
                n_bins = 64
                eta_min, eta_max = -2.0, 2.0
            else:
                n_bins = 64
                if region == 1:
                    eta_min, eta_max = 1.05, 2.4
                else:
                    eta_min, eta_max = -2.4, -1.05
            step = (eta_max - eta_min) / n_bins
            return [eta_min + i * step for i in range(n_bins + 1)]

        elif chamber_type == 'RPC':
            n_bins = 48
            eta_min, eta_max = -2.0, 2.0
            step = (eta_max - eta_min) / n_bins
            return [eta_min + i * step for i in range(n_bins + 1)]

        elif chamber_type == 'TGC':
            if sampling_id == 5:
                n_bins = 64
            else:
                n_bins = 48
            if region == 1:
                eta_min, eta_max = 1.05, 2.4
            else:
                eta_min, eta_max = -2.4, -1.05
            step = (eta_max - eta_min) / n_bins
            return [eta_min + i * step for i in range(n_bins + 1)]

        # Fallback genérico
        n_bins = 64
        eta_min, eta_max = -2.5, 2.5
        step = (eta_max - eta_min) / n_bins
        return [eta_min + i * step for i in range(n_bins + 1)]

    def _get_phi_bins(self, chamber_type):
        """Define os limites dos bins em phi (de -π a +π) para o tipo de
        câmara, usando o número de bins configurado em chamber_properties."""
        if chamber_type in ['MDT', 'RPC', 'TGC']:
            n_bins = self.chamber_properties.get(chamber_type, {}).get('phi_bins', 48)
        else:
            n_bins = 48
        phi_min, phi_max = -3.14159265359, 3.14159265359
        step = (phi_max - phi_min) / n_bins
        return [phi_min + i * step for i in range(n_bins + 1)]

    def _get_r_value(self, chamber_type, sampling_id, region):
        """Retorna o raio médio (em mm) da sampling, usado na definição
        geométrica da célula."""
        if chamber_type == 'MDT':
            if region == 0:
                if sampling_id == 1:
                    return 5000.0
                elif sampling_id == 2:
                    return 7500.0
                else:
                    return 10000.0
            else:
                if sampling_id == 1:
                    return 5500.0
                elif sampling_id == 2:
                    return 6500.0
                else:
                    return 7500.0
        elif chamber_type == 'RPC':
            if sampling_id == 4:
                return 7700.0
            else:
                return 10300.0
        elif chamber_type == 'TGC':
            if sampling_id == 5:
                return 5000.0
            else:
                return 6500.0
        return 5000.0

    def _get_detector_code(self, chamber_type):
        """Mapeia o nome do detector para o código inteiro esperado pelo C++."""
        mapping = {'MDT': 0, 'RPC': 1, 'TGC': 2, 'CSC': 3}
        return mapping.get(chamber_type.upper(), 0)

    def _extract_sampling_properties(self, samp):
        """Extrai todas as propriedades relevantes de uma sampling:
        tipo de câmara, região, station, layer, shaper, samples, ruído,
        range de BCID, pesos do filtro ótimo, raio, bins de eta/phi e
        chave de coleção única."""
        # --- Tipo de câmara (default MDT) ---------------------------------
        chamber_type = "MDT"
        if hasattr(samp, 'Detector'):
            det_str = str(samp.Detector).upper()
            for chamber in ['MDT', 'RPC', 'TGC', 'CSC']:
                if chamber in det_str:
                    chamber_type = chamber
                    break

        # Configuração padrão do tipo de câmara (fallback)
        chamber_config = self.chamber_properties.get(chamber_type, self.chamber_properties['MDT'])

        # --- Região (0=Barrel, 1=EndcapA, 2=EndcapC) ----------------------
        region = 0
        if hasattr(samp, 'Region'):
            region_val = samp.Region
            if isinstance(region_val, str):
                region_str = region_val.upper()
                if "A" in region_str or "POS" in region_str:
                    region = 1
                elif "C" in region_str or "NEG" in region_str:
                    region = 2
                else:
                    region = 0
            else:
                region = int(region_val)

        # Sanity checks: TGC só existe em endcap, RPC só em barrel
        if chamber_type == 'TGC' and region == 0:
            MSG_WARNING(self, f"TGC found with region=0! Forcing to EndcapA.")
            region = 1

        if chamber_type == 'RPC' and region != 0:
            MSG_WARNING(self, f"RPC found with region={region}! Forcing to Barrel.")
            region = 0

        # --- Sampling ID --------------------------------------------------
        sampling_id = 1
        if hasattr(samp, 'Sampling'):
            sampling_id = int(samp.Sampling)
        elif hasattr(samp, 'DetectorElementId'):
            sampling_id = int(samp.DetectorElementId)

        # Nomes legíveis para região e station
        region_name  = self._get_region_name(region)
        station_name = self._get_station_name(chamber_type, sampling_id, region_name)

        # --- Propriedades extraídas da sampling ---------------------------
        properties = {
            'chamber_type':   chamber_type,
            'sampling_id':    sampling_id,
            'station_name':   station_name,
            'layer':          getattr(samp, 'Layer', 1),
            'region':         region,
            'region_name':    region_name,
            'shaper_file':    getattr(samp, 'Shaper', chamber_config['shaper_file']),
            'n_samples':      getattr(samp, 'Samples', chamber_config['default_samples']),
            'noise':          getattr(samp, 'Noise', chamber_config['default_noise']),
            'start_bc':       getattr(samp, 'StartSamplingBC', 0),
            'energy_weights': getattr(samp, 'OFWeightsEnergy', []),
            'time_weights':   getattr(samp, 'OFWeightsTime', []),
            'bunch_start':    getattr(samp, 'BunchIdStart', -20),
            'bunch_end':      getattr(samp, 'BunchIdEnd', 20),
            'r':              self._get_r_value(chamber_type, sampling_id, region),
            'eta_bins':       self._get_eta_bins(chamber_type, sampling_id, region),
            'phi_bins':       self._get_phi_bins(chamber_type),
        }

        # Aplica o fator de ruído global, se ruído estiver habilitado
        if self.doNoise:
            properties['noise'] *= self.noiseFactor

        # Gera uma chave única para a coleção desta sampling
        properties['collection_key'] = self._get_unique_collection_name(
            chamber_type, station_name, region_name
        )

        return properties

    def _create_muon_alg(self, samp_name, properties, samp_obj):
        """Cria a cadeia MuonPulseGenerator + MuonOptimalFilter + MuonCellMaker
        para uma única sampling. Retorna (cell_maker, collection_key)."""
        try:
            chamber_type   = properties['chamber_type']
            sampling_id    = properties['sampling_id']
            region_name    = properties['region_name']
            station_name   = properties['station_name']
            collection_key = properties['collection_key']

            # Nome único do algoritmo e caminho dos histogramas
            alg_name  = self._get_unique_alg_name(chamber_type, sampling_id, region_name, station_name)
            hist_path = f"{self.HistogramPath}/{alg_name}"

            # --- Logs de debug -------------------------------------------
            MSG_DEBUG(self, f"Creating algorithm: {alg_name}")
            MSG_DEBUG(self, f"  Chamber: {chamber_type}, Sampling: {sampling_id}")
            MSG_DEBUG(self, f"  Region: {region_name}, Station: {station_name}")
            MSG_DEBUG(self, f"  Collection: {collection_key}")
            MSG_DEBUG(self, f"  R: {properties['r']} mm")
            MSG_DEBUG(self, f"  Eta bins: {len(properties['eta_bins'])-1}")
            MSG_DEBUG(self, f"  Phi bins: {len(properties['phi_bins'])-1}")

            # --- Gerador de pulso (sinal analógico da câmara) ------------
            pulse_gen = MuonPulseGenerator(
                name=f"MuonPulse_{alg_name}",
                NSamples=properties['n_samples'],
                ShaperFile=properties['shaper_file'],
                OutputLevel=self.OutputLevel,
                SamplingRate=25.0,
                Pedestal=0.0,
                DeformationMean=0.0,
                DeformationStd=0.0,
                NoiseMean=0.0,
                NoiseStd=properties['noise'],
                StartSamplingBC=properties['start_bc'],
            )

            # --- Pesos do filtro ótimo (energia e tempo) -----------------
            energy_weights = properties['energy_weights']
            time_weights   = properties['time_weights']

            # Se a sampling não forneceu pesos, usa valores neutros
            if not energy_weights:
                n = properties['n_samples']
                energy_weights = [1.0 / n] * n

            if not time_weights:
                n = properties['n_samples']
                time_weights = [0.0] * n

            # --- Filtro ótimo --------------------------------------------
            optimal_filter = MuonOptimalFilter(
                name=f"MuonOF_{alg_name}",
                WeightsEnergy=energy_weights,
                WeightsTime=time_weights,
                OutputLevel=self.OutputLevel,
            )

            # ============================================================
            # MUON CELL MAKER - SEM PARÂMETRO "station"!
            # ============================================================
            # Cria o construtor de células propriamente dito
            cell_maker = MuonCellMaker(
                name=alg_name,
                InputHitsKey=self.InputHitsKey,
                OutputCollectionKey=collection_key,
                HistogramPath=hist_path,
                OutputLevel=self.OutputLevel,
                DetailedHistograms=self.doDetailedHistograms,
            )

            # Define os bins de eta e phi (arrays de bordas)
            cell_maker.setProperty("EtaBins", properties['eta_bins'])
            cell_maker.setProperty("PhiBins", properties['phi_bins'])

            # Raio médio e faixa radial (RMin/RMax = 0 → usa R como referência)
            cell_maker.setProperty("R",    properties['r'])
            cell_maker.setProperty("RMin", 0.0)
            cell_maker.setProperty("RMax", 0.0)

            # Identificação geométrica da sampling
            cell_maker.setProperty("Layer",    properties['layer'])
            cell_maker.setProperty("Sampling", sampling_id)
            cell_maker.setProperty("Detector", self._get_detector_code(chamber_type))  # ← "Detector"
            cell_maker.setProperty("Region",   properties['region'])

            # Range de Bunch Crossing IDs e duração do bunch
            cell_maker.setProperty("BunchIdStart", properties['bunch_start'])
            cell_maker.setProperty("BunchIdEnd",   properties['bunch_end'])
            cell_maker.setProperty("BunchDuration", 25.0)

            # Acopla gerador de pulso e ferramentas (filtro ótimo) ao maker
            cell_maker.PulseGenerator = pulse_gen
            cell_maker.Tools = [optimal_filter]

            # Guarda a chave da célula gerada e loga
            muon_cell_key = f"{collection_key}_Cells"
            self.MuonCellKeys.append(muon_cell_key)
            MSG_DEBUG(self, f"  Cell key: {muon_cell_key}")

            return cell_maker, collection_key

        except Exception as e:
            # Em caso de falha, loga o erro e imprime traceback completo
            MSG_ERROR(self, f"Failed to create muon algorithm for {samp_name}: {e}")
            import traceback
            traceback.print_exc()
            return None, None

    def configure(self):
        """Percorre todas as samplings do detector, cria a cadeia de
        reconstrução para cada sampling de muon válida e, ao final, cria
        o MuonCellMerge para unificar tudo."""
        MSG_INFO(self, "Configure MuonCellBuilder.")

        # Sanity check: o detector precisa expor 'samplings'
        if not hasattr(self.Detector, 'samplings'):
            MSG_ERROR(self, "Detector object has no 'samplings' attribute!")
            return

        MSG_INFO(self, f"Found {len(self.Detector.samplings)} total samplings")

        processed_count = 0

        # --- Loop principal sobre todas as samplings ---------------------
        for i, samp in enumerate(self.Detector.samplings):
            try:
                # Ignora samplings que não são de muon
                if not self._is_valid_muon_sampling(samp):
                    continue

                # Extrai nome e propriedades
                samp_name  = self._get_sampling_name_safe(samp)
                properties = self._extract_sampling_properties(samp)

                # Log detalhado da sampling em processamento
                MSG_INFO(self, f"Processing: {samp_name}")
                MSG_INFO(self, f"  Chamber type: {properties['chamber_type']}")
                MSG_INFO(self, f"  Sampling ID: {properties['sampling_id']}")
                MSG_INFO(self, f"  Region: {properties['region']} ({properties['region_name']})")
                MSG_INFO(self, f"  R: {properties['r'] / 1000.0:.1f} m")
                MSG_INFO(self, f"  Eta bins: {len(properties['eta_bins']) - 1}")
                MSG_INFO(self, f"  Phi bins: {len(properties['phi_bins']) - 1}")
                MSG_INFO(self, f"  Samples: {properties['n_samples']}, Noise: {properties['noise']:.3f}")
                MSG_INFO(self, f"  Shaper: {properties['shaper_file']}")

                # Cria a cadeia de algoritmos para esta sampling
                cell_maker, collection_key = self._create_muon_alg(samp_name, properties, samp)

                if cell_maker is not None:
                    self.__recoAlgs.append(cell_maker)
                    self.OutputCollectionKeys.append(collection_key)
                    processed_count += 1
                    MSG_INFO(self, f"  Created collection: {collection_key}")

            except Exception as e:
                # Erros em uma sampling não devem abortar as demais
                MSG_ERROR(self, f"Error processing sampling {i}: {str(e)[:200]}")
                continue

        # --- Merge final (se alguma coleção foi criada) ------------------
        if processed_count > 0:
            MSG_INFO(self, f"Successfully created {processed_count} muon collections")
            MSG_INFO(self, f"Collected {len(self.MuonCellKeys)} muon cell keys")

            try:
                merge_alg = MuonCellMerge(
                    name="MuonCellMerge",
                    InputCollectionKeys=self.OutputCollectionKeys,
                    OutputTruthCellsKey=self.OutputTruthCellsKey,
                    OutputCellsKey=self.OutputCellsKey,
                    OutputLevel=self.OutputLevel,
                )
                self.__recoAlgs.append(merge_alg)
                MSG_INFO(self, "Created MuonCellMerge")
            except Exception as e:
                MSG_ERROR(self, f"Failed to create MuonCellMerge: {e}")
        else:
            MSG_WARNING(self, "No muon collections created!")

    def merge(self, acc):
        """Adiciona todos os algoritmos configurados ao acumulador (acc)
        e imprime um resumo final do que foi construído."""

        if not self.__recoAlgs:
            self.configure()

        if not self.__recoAlgs:
            MSG_WARNING(self, "No muon reconstruction algorithms to add to accumulator")
            return

        # Adiciona cada algoritmo ao acumulador
        for reco in self.__recoAlgs:
            try:
                acc += reco
                MSG_DEBUG(self,
                          f"Added algorithm: {reco.name if hasattr(reco, 'name') else reco.__class__.__name__}")
            except Exception as e:
                MSG_ERROR(self, f"Failed to add algorithm to accumulator: {e}")

        MSG_INFO(self, f"Added {len(self.__recoAlgs)} muon reconstruction algorithms to accumulator")

        # --- Resumo final ------------------------------------------------
        MSG_INFO(self, "=" * 60)
        MSG_INFO(self, "MUON CELL BUILDER SUMMARY")
        MSG_INFO(self, "=" * 60)
        MSG_INFO(self, f"Input hits: {self.InputHitsKey}")
        MSG_INFO(self, f"Output cells: {self.OutputCellsKey}")
        MSG_INFO(self, f"Output truth cells: {self.OutputTruthCellsKey}")
        MSG_INFO(self, f"Collections created: {len(self.OutputCollectionKeys)}")
        MSG_INFO(self, f"Muon cell keys collected: {len(self.MuonCellKeys)}")
        MSG_INFO(self, f"Algorithms configured: {len(self.__recoAlgs)}")

        MSG_INFO(self, "Collections:")
        for i, col in enumerate(self.OutputCollectionKeys):
            MSG_INFO(self, f"  {i + 1:2d}. {col}")

        if self.MuonCellKeys:
            MSG_INFO(self, "Muon cell keys (first 10):")
            for i, key in enumerate(self.MuonCellKeys[:10]):
                MSG_INFO(self, f"  {i + 1:2d}. {key}")
            if len(self.MuonCellKeys) > 10:
                MSG_INFO(self, f"  ... and {len(self.MuonCellKeys) - 10} more")

        MSG_INFO(self, "=" * 60)
