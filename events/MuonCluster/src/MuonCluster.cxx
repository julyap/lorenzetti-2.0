// ============================================================================
// Arquivo: MuonCluster/src/MuonCluster.cxx
// Descrição: Implementação dos métodos da classe xAOD::MuonCluster.
//            - Construtores (default e parametrizado): inicializam todas as
//              variáveis (básicas, energia, qualidade da track, flags,
//              tracking) e chamam clear().
//            - Destrutor: chama clear().
//            - clear(): limpa o container de células e reinicia todas as
//              variáveis (energia, qualidade, flags, tracking).
//            - push_back(cell): adiciona uma célula ao cluster, somando sua
//              energia ao total e atualizando emaxs2 se for a mais energética.
//            - size() e cells(): acesso ao container.
// ============================================================================

#include "MuonCluster/MuonCluster.h"

using namespace xAOD;

//!=====================================================================
// Construtor default: inicializa todas as variáveis e chama clear()
//!=====================================================================
MuonCluster::MuonCluster()
    : Gaugi::EDM(),
      m_emaxs2(0.0),
      m_eta(0.0),
      m_phi(0.0),
      m_deta(0.0),
      m_dphi(0.0),
      m_seed(nullptr),
      m_totalEnergy(0.0),
      m_fracMax(0.0),
      m_chi2(0.0),
      m_qOverPSignificance(0.0),
      m_rhoPrime(0.0),
      m_isVeryLoose(false),
      m_isLoose(false),
      m_isMedium(false),
      m_isTight(false),
      m_quality(-1),
      // NOVAS INICIALIZAÇÕES DE TRACKING
      m_charge(0),
      m_pt(0.0),
      m_p(0.0),
      m_d0(0.0),
      m_z0(0.0),
      m_trackChi2(0.0)
{
    clear();
}

//!=====================================================================
// Construtor parametrizado: inicializa propriedades básicas e chama clear()
//!=====================================================================
MuonCluster::MuonCluster(float emaxs2, float eta, float phi, float deta, float dphi)
    : Gaugi::EDM(),
      m_emaxs2(emaxs2),
      m_eta(eta),
      m_phi(phi),
      m_deta(deta),
      m_dphi(dphi),
      m_seed(nullptr),
      m_totalEnergy(0.0),
      m_fracMax(0.0),
      m_chi2(0.0),
      m_qOverPSignificance(0.0),
      m_rhoPrime(0.0),
      m_isVeryLoose(false),
      m_isLoose(false),
      m_isMedium(false),
      m_isTight(false),
      m_quality(-1),
      // NOVAS INICIALIZAÇÕES DE TRACKING
      m_charge(0),
      m_pt(0.0),
      m_p(0.0),
      m_d0(0.0),
      m_z0(0.0),
      m_trackChi2(0.0)
{
    clear();
}

//!=====================================================================
// Destrutor: libera o container de células
//!=====================================================================
MuonCluster::~MuonCluster()
{
    clear();
}

//!=====================================================================
// clear(): limpa o container de células e reinicia todas as variáveis
//!=====================================================================
void MuonCluster::clear()
{
    m_container.clear();
    m_totalEnergy = 0.0;
    m_emaxs2 = 0.0;
    m_fracMax = 0.0;
    m_chi2 = 0.0;
    m_qOverPSignificance = 0.0;
    m_rhoPrime = 0.0;
    m_isVeryLoose = false;
    m_isLoose = false;
    m_isMedium = false;
    m_isTight = false;
    m_quality = -1;

    // NOVAS VARIÁVEIS DE TRACKING - também resetar no clear()
    m_charge = 0;
    m_pt = 0.0;
    m_p = 0.0;
    m_d0 = 0.0;
    m_z0 = 0.0;
    m_trackChi2 = 0.0;
}

//!=====================================================================
// push_back(cell): adiciona uma célula ao cluster, atualizando a energia
// total e a energia da célula mais energética (emaxs2)
//!=====================================================================
void MuonCluster::push_back(const xAOD::MuonChamberCell* cell)
{
    if (cell) {
        m_container.push_back(cell);
        m_totalEnergy += cell->m_energy;
        if (cell->m_energy > m_emaxs2) {
            m_emaxs2 = cell->m_energy;
        }
    }
}

//!=====================================================================
// size(): número de células no cluster
//!=====================================================================
size_t MuonCluster::size() const
{
    return m_container.size();
}

//!=====================================================================
// cells(): retorna o container de células do cluster
//!=====================================================================
const std::vector<const xAOD::MuonChamberCell*>& MuonCluster::cells() const
{
    return m_container;
}
