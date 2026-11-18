// ============================================================================
// Arquivo: MuonCluster/src/MuonClusterConverter.cxx
// Autora: Julya Pacheco Ferreira
// Descrição: Implementação da classe xAOD::MuonClusterConverter.
//            - convert(const MuonCluster*, MuonCluster_t&): serializa as
//              propriedades básicas, qualidade da track e energia para a
//              struct POD (sem links de células).
//            - convertWithIndices(...): serializa também os links de
//              células e seed, usando índices (evita ponteiros na
//              persistência).
//            - convert(const MuonCluster_t&, MuonCluster*&): desserializa
//              criando um novo MuonCluster com as propriedades básicas.
//            - resolveCells(...): reconstrói os links de células e seed
//              a partir dos índices persistidos.
# ============================================================================

#include "MuonCluster/MuonClusterConverter.h"
#include "MuonCluster/MuonCluster.h"

namespace xAOD {

//!=====================================================================
// Serialização básica: MuonCluster* → MuonCluster_t
//   - Copia propriedades básicas (eta, phi, deta, dphi, emaxs2)
//   - Copia qualidade da track (chi2, qOverPSignificance, rhoPrime)
//   - Copia energia (totalEnergy, fracMax)
//   - Limpa os índices (serão preenchidos por convertWithIndices)
//!=====================================================================
bool MuonClusterConverter::convert(const MuonCluster *cluster, MuonCluster_t &cluster_t)
{
    if (!cluster) {
        return false;
    }

    // Basic cluster properties
    cluster_t.eta = cluster->eta();
    cluster_t.phi = cluster->phi();
    cluster_t.deta = cluster->deta();
    cluster_t.dphi = cluster->dphi();
    cluster_t.emaxs2 = cluster->emaxs2();

    // Track quality variables
    cluster_t.chi2 = cluster->chi2();
    cluster_t.qOverPSignificance = cluster->qOverPSignificance();
    cluster_t.rhoPrime = cluster->rhoPrime();

    // Energy information
    cluster_t.totalEnergy = cluster->totalEnergy();
    cluster_t.fracMax = cluster->fracMax();

    // Clear indices
    cluster_t.chamber_indices.clear();
    cluster_t.seed_index = 0;

    return true;
}

//!=====================================================================
// Serialização com índices: MuonCluster* → MuonCluster_t
//   - Chama convert() para propriedades básicas
//   - Preenche chamber_indices usando o mapa cell→índice
//   - Preenche seed_index usando o mesmo mapa
//!=====================================================================
bool MuonClusterConverter::convertWithIndices(const MuonCluster *cluster, MuonCluster_t &cluster_t,
                                              const std::map<const MuonChamberCell*, size_t>& cell_index_map)
{
    if (!cluster) {
        return false;
    }

    // Convert basic properties
    if (!convert(cluster, cluster_t)) {
        return false;
    }

    // Convert cell links using indices
    for (const auto cell : cluster->cells()) {
        auto it = cell_index_map.find(cell);
        if (it != cell_index_map.end()) {
            cluster_t.chamber_indices.push_back(it->second);
        }
    }

    // Convert seed link
    const MuonChamberCell* seed = cluster->seed();
    if (seed) {
        auto it = cell_index_map.find(seed);
        if (it != cell_index_map.end()) {
            cluster_t.seed_index = it->second;
        }
    }

    return true;
}

//!=====================================================================
// Desserialização básica: MuonCluster_t → MuonCluster*
//   - Cria um novo MuonCluster com as propriedades básicas
//   - Configura qualidade da track e energia
//!=====================================================================
bool MuonClusterConverter::convert(const MuonCluster_t &cluster_t, MuonCluster *&cluster)
{
    // Create a new MuonCluster
    cluster = new MuonCluster(cluster_t.emaxs2, cluster_t.eta, cluster_t.phi,
                               cluster_t.deta, cluster_t.dphi);

    if (!cluster) {
        return false;
    }

    // Set track quality variables
    cluster->setChi2(cluster_t.chi2);
    cluster->setQOverPSignificance(cluster_t.qOverPSignificance);
    cluster->setRhoPrime(cluster_t.rhoPrime);

    // Set energy information
    cluster->setTotalEnergy(cluster_t.totalEnergy);
    cluster->setFracMax(cluster_t.fracMax);

    return true;
}

//!=====================================================================
// Resolução de células: preenche cluster->cells() e cluster->seed() a
// partir dos índices persistidos em cluster_t
//!=====================================================================
bool MuonClusterConverter::resolveCells(MuonCluster *cluster,
                                        const std::vector<const MuonChamberCell*>& cells_by_index,
                                        const MuonCluster_t &cluster_t)
{
    if (!cluster) {
        return false;
    }

    // Clear existing cells
    cluster->clear();

    // Add cells by index
    for (size_t idx : cluster_t.chamber_indices) {
        if (idx < cells_by_index.size() && cells_by_index[idx]) {
            cluster->push_back(cells_by_index[idx]);
        }
    }

    // Set seed by index
    if (cluster_t.seed_index < cells_by_index.size() && cells_by_index[cluster_t.seed_index]) {
        cluster->setSeed(cells_by_index[cluster_t.seed_index]);
    }

    return true;
}

} // namespace xAOD
