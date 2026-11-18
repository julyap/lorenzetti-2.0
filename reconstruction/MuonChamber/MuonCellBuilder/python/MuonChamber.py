# ============================================================================
# Arquivo: MuonChamber.py
# Descrição: Classe leve que representa uma câmara de muon (uma "sensitive
#            volume" já existente na geometria). Armazena propriedades
#            relevantes para a simulação/reconstrução (chave de coleção,
#            shaper, ruído, número de amostras, pesos do filtro ótimo,
#            segmentação X/Y) e expõe métodos utilitários para acessar o
#            volume e o sensitive associado.
# ============================================================================

__all__ = ["MuonChamber"]

# --- Imports do framework ---------------------------------------------------
from GaugiKernel        import Logger


class MuonChamber(Logger):
    """
    Representa uma câmara de muon a partir de um sensitive volume da
    geometria. Herda de Logger para emitir mensagens padronizadas.
    """

    # Lista de chaves permitidas ao construir a câmara.
    # Qualquer kwarg fora dessa lista dispara MSG_FATAL.
    __allow_keys = ["CollectionKey",
                    "DetectorElementId",
                    "Shaper",
                    "Noise",
                    "Samples",
                    "OFWeightsEnergy",
                    "OFWeightsTime",
                    "XSegments",
                    "YSegments",
                    ]

    def __init__(self, sv, bunch_start, bunch_end, sampling_start, **kw):
        # sv             = sensitive volume da geometria
        # bunch_start    = primeiro BCID considerado
        # bunch_end      = último BCID considerado
        # sampling_start = BCID inicial de amostragem
        # kw             = propriedades extras (validadas por __allow_keys)

        # Inicializa o Logger
        Logger.__init__(self)

        # Valida e atribui cada kwarg como atributo, se permitido
        for key, value in kw.items():
            if key in self.__allow_keys:
                setattr(self, key, value )
            else:
                MSG_FATAL( self, "Property with name %s is not allow for %s object", key, self.__class__.__name__)

        # Guarda as referências básicas
        self.sv              = sv
        self.BunchIdStart    = bunch_start
        self.BunchIdEnd      = bunch_end
        self.StartSamplingBC = sampling_start

    def volume(self):
        """Retorna o volume geométrico associado ao sensitive."""
        return self.sv.volume()

    def sensitive(self):
        """Retorna o próprio sensitive volume."""
        return self.sv

    def name(self):
        """Retorna um nome único combinando o nome do volume e o
        DetectorElementId."""
        return self.volume().name() + '_' + str(self.DetectorElementId)
