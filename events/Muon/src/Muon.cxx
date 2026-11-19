// ============================================================================
// Arquivo: Muon.cxx
// Descrição: Implementação dos construtores da classe xAOD::Muon.
//            - O construtor padrão inicializa todas as variáveis em zero
//              (e/et/pt/eta/phi/carga/qualidade/iso/d0/z0/σd0/σz0/fracMax/
//              chi2/qOverPSignificance/rhoPrime) e o ponteiro para o cluster
//              como nullptr.
//            - O construtor parametrizado (eta, phi, pt) calcula a energia
//              como E = pT·cosh(η) e inicializa as demais variáveis em zero,
//              deixando o ponteiro para o cluster como nullptr.
// ============================================================================

#include "Muon/Muon.h"

using namespace xAOD;

//!=====================================================================
// Construtor padrão: inicializa todas as variáveis em zero
//!=====================================================================
Muon::Muon() :
  EDM(),
  m_e(0),
  m_et(0),
  m_pt(0),
  m_eta(0),
  m_phi(0),
  m_charge(0),
  m_quality(0),
  m_iso(0),
  m_d0(0),
  m_z0(0),
  m_sigmaD0(0),
  m_sigmaZ0(0),
  m_fracMax(0),
  m_chi2(0),
  m_qOverPSignificance(0),
  m_rhoPrime(0),
  m_muonCluster(nullptr)
{}

//!=====================================================================
// Construtor parametrizado (eta, phi, pt):
//   - E  = pT·cosh(η)  (para partículas sem massa)
//   - ET = pT
//   - As demais variáveis de qualidade/impacto começam em zero
//   - O ponteiro para o cluster começa como nullptr
//!=====================================================================
Muon::Muon(float eta, float phi, float pt) :
  EDM(),
  m_e(pt * std::cosh(eta)),   // E = pT·cosh(η)
  m_et(pt),                    // ET = pT
  m_pt(pt),
  m_eta(eta),
  m_phi(phi),
  m_charge(0),
  m_quality(0),
  m_iso(0),
  m_d0(0),
  m_z0(0),
  m_sigmaD0(0),
  m_sigmaZ0(0),
  m_fracMax(0),
  m_chi2(0),
  m_qOverPSignificance(0),
  m_rhoPrime(0),
  m_muonCluster(nullptr)
{}
