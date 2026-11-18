// ============================================================================
// Arquivo: MuonCluster/MuonCluster/MuonClusterConverter.h
// Autora: Julya Pacheco Ferreira
// Descrição: Define:
//              - a struct POD xAOD::MuonCluster_t, usada para persistir
//                clusters em arquivos ROOT com campos primitivos;
//              - o typedef muon_cluster_links_t (mapa de índices para
//                MuonCluster*);
//              - a classe MuonClusterConverter, responsável por converter
//                entre xAOD::MuonCluster* e MuonCluster_t, incluindo
//                conversão com índices (para evitar ponteiros na
//                persistência) e resolução de células a partir de índices.
// ============================================================================

#ifndef MuonClusterConverter_h
#define MuonClusterConverter_h

/** simulator includes **/
#include "MuonCluster/MuonCluster.h"
#include "MuonCell/MuonChamberConverter.h"
#include "EventInfo/SeedConverter.h"
#include <map>
#include <vector>

namespace xAOD{
    // Mapa de índices para clusters (usado para resolver links)
    typedef std::map<int, const xAOD::MuonCluster*> muon_cluster_links_t;

    // --- Struct POD serializável (usada para persistência em ROOT) ---
    struct MuonCluster_t{
        // Basic cluster properties
        float eta;
        float phi;
        float deta;
        float dphi;
        float emaxs2;

        // Track quality variables
        float chi2;
        float qOverPSignificance;
        float rhoPrime;

        // Energy information
        float totalEnergy;
        float fracMax;

        // Links to cells and seed
        // (índices no vetor de células do evento)
        std::vector<size_t> chamber_indices;
        size_t seed_index;
    };

    class MuonClusterConverter{
        public:
            MuonClusterConverter()=default;
            ~MuonClusterConverter()=default;

            // Serialização / desserialização básica
            bool convert(const MuonCluster *, MuonCluster_t &);
            bool convert(const MuonCluster_t &, MuonCluster *&);

            // Conversão com mapeamento de índices (para persistência)
            bool convertWithIndices(const MuonCluster *, MuonCluster_t &,
                                   const std::map<const MuonChamberCell*, size_t>& cell_index_map);

            // Resolução de células a partir de índices
            bool resolveCells(MuonCluster *cluster,
                            const std::vector<const MuonChamberCell*>& cells_by_index,
                            const MuonCluster_t &cluster_t);
    };
}

#endif
