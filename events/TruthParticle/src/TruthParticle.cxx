// TruthParticle.cxx - CORRIGIDO COM NOVAS VARIÁVEIS
#include "TruthParticle/TruthParticle.h"

using namespace xAOD;

TruthParticle::TruthParticle():
  m_pdgid(0),
  m_seedid(0),
  m_e(0),
  m_et(0),
  m_eta(0),
  m_phi(0),
  m_px(0),
  m_py(0),
  m_pz(0),
  m_vtx_x(0),
  m_vtx_y(0),
  m_vtx_z(0),
  // ============================================================
  // INICIALIZAR AS NOVAS VARIÁVEIS
  // ============================================================
  m_pair_pdg1(0),
  m_pair_pdg2(0),
  m_pair_pt1(0),
  m_pair_pt2(0),
  m_pair_eta1(0),
  m_pair_eta2(0),
  m_pair_phi1(0),
  m_pair_phi2(0),
  m_pair_delta_eta(0),
  m_pair_delta_phi(0),
  m_pair_delta_R(0),
  m_pair_mass(0),
  m_pair_pt(0),
  m_pair_eta(0),
  m_pair_phi(0),
  m_pair_seedid(-1)
{;}

// ============================================================
// CORREÇÃO: Ordem dos parâmetros agora é (pdgid, seedid)
// para corresponder ao TruthParticle.h
// ============================================================
TruthParticle::TruthParticle( int pdgid, int seedid, 
                              float e, float et, float eta, float phi , 
                              float px, float py, float pz,  
                              float vx, float vy, float vz ):
                                
  m_pdgid(pdgid),    // Agora pdgid recebe o primeiro argumento
  m_seedid(seedid),  // Agora seedid recebe o segundo argumento
  m_e(e),
  m_et(et),
  m_eta(eta),
  m_phi(phi),
  m_px(px),
  m_py(py),
  m_pz(pz),
  m_vtx_x(vx),
  m_vtx_y(vy),
  m_vtx_z(vz),
  // ============================================================
  // INICIALIZAR AS NOVAS VARIÁVEIS COM VALORES PADRÃO
  // ============================================================
  m_pair_pdg1(0),
  m_pair_pdg2(0),
  m_pair_pt1(0),
  m_pair_pt2(0),
  m_pair_eta1(0),
  m_pair_eta2(0),
  m_pair_phi1(0),
  m_pair_phi2(0),
  m_pair_delta_eta(0),
  m_pair_delta_phi(0),
  m_pair_delta_R(0),
  m_pair_mass(0),
  m_pair_pt(0),
  m_pair_eta(0),
  m_pair_phi(0),
  m_pair_seedid(-1)
{;}
