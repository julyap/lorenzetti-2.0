# ============================================================================
# Arquivo: CutBasedMuonID.py
# Autora: Julya Pacheco Ferreira
# Descrição: Define os cortes de identificação de múons baseados em critérios
#            de qualidade (cut-based), inspirados na performance de
#            reconstrução de múons do ATLAS em colisões pp a sqrt(s)=13 TeV
#            (Eur. Phys. J. C 76 (2016) 292, arXiv:1603.05598).
#
#            Organiza os cortes por working point:
#              - vloose (Very Loose)
#              - loose  (Loose)
#              - medium (Medium)
#              - tight  (Tight-like: aproximação para MS-only, NÃO é o Tight
#                        oficial do ATLAS, que exige combinação ID+MS)
#              - highpt (High-pT)
#              - lowpt  (Low-pT)
#
#            Cada working point define valores para:
#              - compatibilidade q/p (qpCompatibilityCut);
#              - ρ' (rhoCut);
#              - χ²/ndof (chi2Cut);
#              - nº mínimo de precision stations (precisionStationsCut);
#              - MBS, SNS, SCS (mbsCut, snsCut, scsCut);
#              - fracMax (fracMaxCut);
#              - nº mínimo de hits no pixel e no SCT, e máximo de hits perdidos;
#              - significâncias de d0 e z0·sinθ (vertex association);
#              - tipos de múon aceitos (CB, IO, ME, SiF, ST, CT);
#              - thresholds em pT.
#
#            Também expõe métodos utilitários (getCuts, getCutsDict) e métodos
#            de compatibilidade (getEratioCuts, getRetaCuts, getRphiCuts,
#            getVLooseCuts, getLooseCuts, getMediumCuts, getTightCuts) para
#            manter interface com código existente.
# ============================================================================

__all__ = ['CutBasedMuonID']


class CutBasedMuonID:
    def __init__(self):
        """
        Baseado no artigo ATLAS:
        "Muon reconstruction performance of the ATLAS detector in proton-proton
        collision data at sqrt(s)=13 TeV"
        Eur. Phys. J. C 76 (2016) 292, arXiv:1603.05598

        Referência das páginas:
        - Seção 5: Muon identification (páginas 6-8)
        - Tabela 1: Efficiency for prompt muons (página 8)
        - Seção 5.1.2: Quality requirements for CB muons (página 6)
        - Seção 5.1.4: Low-pT specific cuts (página 7)
        - Seção 5.2: Vertex association (página 8)

        NOTA: O working point 'tight' aqui definido é uma aproximação
        "Tight-like" para múons MS-only. O Tight oficial do ATLAS exige
        combinação ID+MS e não é reproduzido apenas com cortes MS-only.
        """

        # ================================================================
        # q/p compatibility cuts (|q/p_ID - q/p_MS| / sqrt(σ²_ID + σ²_MS))
        # Seção 5.1.2 (página 6): "q/p significance is required to be less than seven"
        # ================================================================
        self.qpCompatibilityCut = {
            'vloose': [10.0],     # Very loose: muito frouxo
            'loose': [8.0],       # Loose: mais tolerante
            'medium': [7.0],      # Medium: q/p significance < 7 (página 6)
            'tight': [5.0],       # Tight-like: mais restritivo para baixo pT
            'highpt': [5.0],      # High-pT: otimizado para resolução
            'lowpt': [7.0],       # Low-pT: mesmo do Medium
        }

        # ================================================================
        # Rho' cuts (|pT_ID - pT_MS| / pT_CB)
        # Seção 5.1.2 (página 6-7): usado no Tight WP para rejeitar background
        # "A two-dimensional cut in the ρ′ and q/p significance variables"
        # ================================================================
        self.rhoCut = {
            'vloose': [1.5],      # Very loose: muito frouxo
            'loose': [1.0],       # Loose: cortes frouxos
            'medium': [1.0],      # Medium: sem corte explícito de ρ'
            'tight': [0.3],       # Tight-like: ρ' < 0.3 (página 7)
            'highpt': [0.5],      # High-pT: ρ' < 0.5
            'lowpt': [1.0],       # Low-pT: usa MBS/SNS/SCS ao invés de ρ'
        }

        # ================================================================
        # χ²/ndof do combined track fit
        # Seção 5.1.2 (página 7): "normalized χ² of the combined track fit
        # is required to be < 8 to remove pathological tracks"
        # ================================================================
        self.chi2Cut = {
            'vloose': [12.0],     # Very loose: mais tolerante
            'loose': [10.0],
            'medium': [10.0],
            'tight': [8.0],       # Tight-like: χ²/ndof < 8 (página 7)
            'highpt': [8.0],
            'lowpt': [10.0],
        }

        # ================================================================
        # Mínimo de precision stations (câmaras MDT/CSC com ≥3 hits)
        # Seção 5.1.1 (página 6): "≥3 hits in at least two MDT layers"
        # ================================================================
        self.precisionStationsCut = {
            'vloose': [1],        # Very loose: aceita 1 estação
            'loose': [1],         # Loose: aceita 1 estação
            'medium': [2],        # Medium: ≥2 estações (página 6)
            'tight': [2],         # Tight-like: ≥2 estações (página 7)
            'highpt': [3],        # High-pT: ≥3 estações (página 7)
            'lowpt': [1],         # Low-pT: ≥1 estação
        }

        # ================================================================
        # Low-pT specific cuts: kink variables (Seção 5.1.4, página 7)
        # MBS, SNS, SCS - usados para rejeitar decaimentos de hádrons
        # "Momentum Balance Significance (MBS), Scattering Neighbor Significance (SNS),
        #  and Scattering Curvature Significance (SCS)"
        # ================================================================
        self.mbsCut = {           # Momentum Balance Significance
            'vloose': [10.0],
            'loose': [10.0],
            'medium': [10.0],
            'tight': [10.0],
            'highpt': [10.0],
            'lowpt': [3.0],       # MBS < 3 (página 7)
        }

        self.snsCut = {           # Scattering Neighbor Significance
            'vloose': [10.0],
            'loose': [10.0],
            'medium': [10.0],
            'tight': [10.0],
            'highpt': [10.0],
            'lowpt': [3.0],       # SNS < 3 (página 7)
        }

        self.scsCut = {           # Scattering Curvature Significance
            'vloose': [10.0],
            'loose': [10.0],
            'medium': [10.0],
            'tight': [10.0],
            'highpt': [10.0],
            'lowpt': [3.0],       # SCS < 3 (página 7)
        }

        # ================================================================
        # FracMax cuts (energy concentration in hottest cell)
        # Múons são MIPs: depositam energia em poucas células
        # Quanto maior o fracMax, mais concentrada a energia
        # ================================================================
        self.fracMaxCut = {
            'vloose': [0.05],     # Very loose: mínimo de concentração
            'loose': [0.10],      # Loose: pelo menos 10% na célula principal
            'medium': [0.20],     # Medium: pelo menos 20% na célula principal
            'tight': [0.30],      # Tight-like: pelo menos 30% na célula principal
            'highpt': [0.30],
            'lowpt': [0.20],
        }

        # ================================================================
        # ID track quality cuts (Seção 5.1.1, página 6)
        # ================================================================
        self.minPixelHitsCut = {
            'vloose': [1],        # ≥1 hit no pixel detector (página 6)
            'loose': [1],
            'medium': [1],
            'tight': [1],
            'highpt': [1],
            'lowpt': [1],
        }

        self.minSctHitsCut = {
            'vloose': [4],        # ≥4 hits no SCT
            'loose': [5],         # ≥5 hits no SCT (página 6)
            'medium': [5],
            'tight': [5],
            'highpt': [5],
            'lowpt': [5],
        }

        self.maxMissingHitsCut = {
            'vloose': [3],        # ≤3 hits perdidos
            'loose': [2],         # ≤2 hits perdidos (página 6)
            'medium': [2],
            'tight': [2],
            'highpt': [2],
            'lowpt': [2],
        }

        # ================================================================
        # Vertex association cuts (Seção 5.2, página 8)
        # ================================================================
        self.d0SignificanceCut = {
            'vloose': [5.0],      # |d0/σ(d0)| < 5
            'loose': [4.0],       # |d0/σ(d0)| < 4
            'medium': [3.0],      # |d0/σ(d0)| < 3 (página 8)
            'tight': [3.0],
            'highpt': [3.0],
            'lowpt': [3.0],
        }

        self.z0SinThetaCut = {
            'vloose': [1.0],      # |z0|*sinθ < 1.0 mm
            'loose': [0.75],      # |z0|*sinθ < 0.75 mm
            'medium': [0.5],      # |z0|*sinθ < 0.5 mm (página 8)
            'tight': [0.5],
            'highpt': [0.5],
            'lowpt': [0.5],
        }

        # ================================================================
        # Muon type acceptance (Seção 4.2, página 4)
        # CB=Combined, IO=Inside-Out, ME=MS-Extrapolated,
        # ST=Segment-Tagged, CT=Calorimeter-Tagged, SiF=Silicon-associated Forward
        # ================================================================
        self.acceptedMuonTypes = {
            # Very loose: aceita todos os tipos (máxima eficiência)
            'vloose': ['CB', 'IO', 'ME', 'SiF', 'ST', 'CT'],
            # Loose: aceita todos os tipos (página 7)
            'loose': ['CB', 'IO', 'ME', 'SiF', 'ST', 'CT'],
            # Medium: CB/IO no ID acceptance, ME/SiF no forward (página 6)
            'medium': ['CB', 'IO', 'ME', 'SiF'],
            # Tight-like: apenas CB e IO com ≥2 precision stations (página 7)
            'tight': ['CB', 'IO'],
            # High-pT: apenas CB e IO (página 7)
            'highpt': ['CB', 'IO'],
            # Low-pT: apenas CB e IO
            'lowpt': ['CB', 'IO'],
        }

        # ================================================================
        # pT thresholds and ranges (GeV)
        # ================================================================
        self.pTThresholds = {
            'vloose_min': 2.0,        # Very loose: pT > 2 GeV
            'loose_min': 3.0,         # Loose: pT > 3 GeV
            'medium_min': 4.0,        # Medium: pT > 4 GeV
            'tight_min': 5.0,         # Tight-like: pT > 5 GeV
            'lowpt_min': 3.0,         # Low-pT começa em 3 GeV
            'lowpt_max': 20.0,        # Low-pT vai até 20 GeV
            'highpt_min': 100.0,      # High-pT otimizado para >100 GeV
        }

    def getCuts(self, workingPoint):
        """
        Retorna lista de cortes para o working point especificado.
        Baseado no ATLAS 2016 (arXiv:1603.05598)

        Args:
            workingPoint: string - 'vloose', 'loose', 'medium', 'tight', 'highpt', or 'lowpt'

        Returns:
            list: [qpCompatibility, rho, chi2, precisionStations, mbs, sns, scs,
                   fracMax, minPixelHits, minSctHits, maxMissingHits,
                   d0Significance, z0SinTheta, acceptedMuonTypes, pTThresholds]
        """
        wp = workingPoint.lower()

        # Validação do working point
        valid_wps = ['vloose', 'loose', 'medium', 'tight', 'highpt', 'lowpt']
        if wp not in valid_wps:
            raise ValueError(f"Working point '{wp}' not recognized. "
                           f"Available: {valid_wps}")

        # Retorna a lista de cortes na ordem documentada no docstring
        return [
            self.qpCompatibilityCut[wp],      # 0: q/p compatibility (página 6)
            self.rhoCut[wp],                  # 1: ρ' (página 7)
            self.chi2Cut[wp],                 # 2: χ²/ndof (página 7)
            self.precisionStationsCut[wp],    # 3: min precision stations (página 6)
            self.mbsCut[wp],                  # 4: MBS (página 7)
            self.snsCut[wp],                  # 5: SNS (página 7)
            self.scsCut[wp],                  # 6: SCS (página 7)
            self.fracMaxCut[wp],              # 7: fracMax (energy concentration)
            self.minPixelHitsCut[wp],         # 8: min pixel hits (página 6)
            self.minSctHitsCut[wp],           # 9: min SCT hits (página 6)
            self.maxMissingHitsCut[wp],       # 10: max missing hits (página 6)
            self.d0SignificanceCut[wp],       # 11: |d0/σ(d0)| (página 8)
            self.z0SinThetaCut[wp],           # 12: |z0|*sinθ (página 8)
            self.acceptedMuonTypes[wp],       # 13: list of accepted muon types
            self.pTThresholds,                # 14: pT thresholds dict
        ]

    def getCutsDict(self, workingPoint):
        """
        Versão alternativa que retorna um dicionário com nome dos cortes.

        Args:
            workingPoint: string - 'vloose', 'loose', 'medium', 'tight', 'highpt', or 'lowpt'

        Returns:
            dict: Dicionário com todos os cortes nomeados
        """
        wp = workingPoint.lower()

        # Validação do working point
        valid_wps = ['vloose', 'loose', 'medium', 'tight', 'highpt', 'lowpt']
        if wp not in valid_wps:
            raise ValueError(f"Working point '{wp}' not recognized. "
                           f"Available: {valid_wps}")

        # Retorna os cortes nomeados (útil para logs/debug)
        return {
            'qp_compatibility': self.qpCompatibilityCut[wp][0],
            'rho': self.rhoCut[wp][0],
            'chi2_ndof': self.chi2Cut[wp][0],
            'min_precision_stations': self.precisionStationsCut[wp][0],
            'MBS': self.mbsCut[wp][0],
            'SNS': self.snsCut[wp][0],
            'SCS': self.scsCut[wp][0],
            'frac_max': self.fracMaxCut[wp][0],
            'min_pixel_hits': self.minPixelHitsCut[wp][0],
            'min_sct_hits': self.minSctHitsCut[wp][0],
            'max_missing_hits': self.maxMissingHitsCut[wp][0],
            'd0_significance': self.d0SignificanceCut[wp][0],
            'z0_sin_theta': self.z0SinThetaCut[wp][0],
            'accepted_muon_types': self.acceptedMuonTypes[wp],
            'pT_thresholds': self.pTThresholds,
        }

    # ================================================================
    # MÉTODOS DE COMPATIBILIDADE (para manter interface com código existente)
    # ================================================================

    def getEratioCuts(self):
        """
        RETORNA CORTES FRACMAX (substituindo eratio)
        Múons usam fracMax (concentração de energia) em vez de eratio
        """
        return [self.fracMaxCut['vloose'][0], self.fracMaxCut['loose'][0],
                self.fracMaxCut['medium'][0], self.fracMaxCut['tight'][0]]

    def getRetaCuts(self):
        """
        RETORNA CORTES CHI2 (substituindo reta)
        """
        return [self.chi2Cut['vloose'][0], self.chi2Cut['loose'][0],
                self.chi2Cut['medium'][0], self.chi2Cut['tight'][0]]

    def getRphiCuts(self):
        """
        RETORNA CORTES Q/P SIGNIFICANCE (substituindo rphi)
        """
        return [self.qpCompatibilityCut['vloose'][0], self.qpCompatibilityCut['loose'][0],
                self.qpCompatibilityCut['medium'][0], self.qpCompatibilityCut['tight'][0]]

    # Métodos para manter compatibilidade com código existente
    def getVLooseCuts(self):
        return [self.qpCompatibilityCut['vloose'][0], self.rhoCut['vloose'][0], self.chi2Cut['vloose'][0]]

    def getLooseCuts(self):
        return [self.qpCompatibilityCut['loose'][0], self.rhoCut['loose'][0], self.chi2Cut['loose'][0]]

    def getMediumCuts(self):
        return [self.qpCompatibilityCut['medium'][0], self.rhoCut['medium'][0], self.chi2Cut['medium'][0]]

    def getTightCuts(self):
        return [self.qpCompatibilityCut['tight'][0], self.rhoCut['tight'][0], self.chi2Cut['tight'][0]]
