#include <cmath>
#include "Jpsi.h"
#include "helper.h"

using namespace generator;

Jpsi::Jpsi(const std::string& name, IGenerator* gen)
  : IMsgService(name),
    IAlgorithm(gen)
{
  declareProperty("EtaMax", m_etaMax = 2.5);
  declareProperty("MinPt", m_minPt = 0.0);
  declareProperty("ZeroVertexParticles", m_zeroVertexParticles = false);
}

Jpsi::~Jpsi() {}

StatusCode Jpsi::initialize()
{
  setMsgLevel(m_outputLevel);
  if (generator()->initialize().isFailure()) {
    MSG_FATAL("Unable to initialize the Jpsi generator. Abort!");
  }
  return StatusCode::SUCCESS;
}

StatusCode Jpsi::execute(generator::Event& ctx)
{
  HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::MM);

  if (generator()->execute(evt).isFailure()) {
    return StatusCode::FAILURE;
  }

  const auto main_event_t = sample_t();
  const auto main_event_z = sample_z();

  // DEBUG: Contar partículas no evento
  int total_particles = 0;
  int n_jpsi = 0;
  int n_charm = 0;
  int n_muons = 0;
  
  for (auto part : evt.particles()) {
    total_particles++;
    int abs_pid = std::abs(part->pid());
    
    if (part->pid() == 443) n_jpsi++;
    if (abs_pid >= 411 && abs_pid <= 445) n_charm++;
    if (abs_pid == 13) n_muons++;
  }
  
  MSG_INFO("Event " << evt.event_number() 
           << ": Total particles=" << total_particles
           << ", J/ψ=" << n_jpsi 
           << ", Charm hadrons=" << n_charm
           << ", Muons=" << n_muons);

  // Procurar o J/ψ
  std::vector<const HepMC3::GenParticle*> jpsi_particles;
  std::vector<const HepMC3::GenParticle*> muon_pairs;

  // Primeiro, encontrar todos os J/ψ
  for (auto part : evt.particles()) {
    if (part->pid() == 443) {
      jpsi_particles.push_back(part.get());
      MSG_DEBUG("Found J/ψ with pT=" << part->momentum().pt() 
                << " GeV, eta=" << part->momentum().eta());
    }
  }

  if (jpsi_particles.empty()) {
    MSG_DEBUG("No Jpsi event inside this event");
    throw NotInterestingEvent();
  }

  // Para cada J/ψ, verificar seus decaimentos
  for (auto jpsi : jpsi_particles) {
    std::vector<const HepMC3::GenParticle*> muons_from_this_jpsi;
    
    int n_children = 0;
    for (auto child : jpsi->children()) {
      n_children++;
      MSG_DEBUG("J/ψ child: PDG=" << child->pid() 
                << ", pT=" << child->momentum().pt()
                << ", eta=" << child->momentum().eta());
      
      if (child->abs_pid() == 13) {
        float eta = child->momentum().eta();
        float pt  = child->momentum().pt();

        if (std::abs(eta) < m_etaMax && pt > m_minPt)
        {
          muons_from_this_jpsi.push_back(child.get());
          MSG_DEBUG(
            "  -> Muon PASSED cuts "
            << "(eta=" << eta
            << ", pt=" << pt
            << ", |eta| < " << m_etaMax
            << ")"
          );
        }
        else
        {
          MSG_DEBUG(
            "  -> Muon FAILED cuts "
            << "(eta=" << eta
            << ", pt=" << pt
            << ", required: |eta| < " << m_etaMax
            << ", pt > " << m_minPt
            << ")"
          );
        }
      }
    }
    
    MSG_DEBUG("J/ψ has " << n_children << " children, " 
              << muons_from_this_jpsi.size() << " muons passing cuts");
    
    if (muons_from_this_jpsi.size() == 2) {
      if ((muons_from_this_jpsi[0]->pid() == 13 && muons_from_this_jpsi[1]->pid() == -13) ||
          (muons_from_this_jpsi[0]->pid() == -13 && muons_from_this_jpsi[1]->pid() == 13)) {
        muon_pairs.insert(muon_pairs.end(), muons_from_this_jpsi.begin(), muons_from_this_jpsi.end());
        MSG_INFO("Found J/psi -> μ⁺μ⁻ with both muons passing cuts");
      } else {
        MSG_DEBUG("J/psi has 2 muons but same charge? " 
                  << muons_from_this_jpsi[0]->pid() << ", " << muons_from_this_jpsi[1]->pid());
      }
    } else {
      MSG_DEBUG("J/psi does NOT have exactly 2 muons (has " 
                << muons_from_this_jpsi.size() << " muons passing cuts)");
    }
  }

  if (muon_pairs.empty()) {
    MSG_DEBUG("No valid J/psi -> μ⁺μ⁻ pair found");
    throw NotInterestingEvent();
  }

  // ============================================================
  // SEED COMPOSTO COM TODAS AS INFORMAÇÕES
  // ============================================================
  
  if (muon_pairs.size() == 2) {
    // Pega os dois múons
    auto* mu1 = muon_pairs[0];
    auto* mu2 = muon_pairs[1];
    
    // Pega os vértices de produção
    const auto vtx1 = mu1->production_vertex();
    const auto vtx2 = mu2->production_vertex();
    
    // Calcula as posições dos vértices em z
    float zVertexPos1 = vtx1->position().pz() + main_event_z;
    float zVertexPos2 = vtx2->position().pz() + main_event_z;
    
    if (m_zeroVertexParticles) {
      zVertexPos1 = vtx1->position().pz();
      zVertexPos2 = vtx2->position().pz();
    }
    
    // ============================================================
    // CÁLCULO DE Δη, Δφ, ΔR
    // ============================================================
    float eta1 = mu1->momentum().eta();
    float eta2 = mu2->momentum().eta();
    float phi1 = mu1->momentum().phi();
    float phi2 = mu2->momentum().phi();
    
    float delta_eta = eta1 - eta2;
    float delta_phi = phi1 - phi2;
    
    // Corrige delta phi para o intervalo [-π, π]
    if (delta_phi > M_PI) delta_phi -= 2 * M_PI;
    if (delta_phi < -M_PI) delta_phi += 2 * M_PI;
    
    float delta_R = std::sqrt(delta_eta * delta_eta + delta_phi * delta_phi);
    
    // ============================================================
    // CÁLCULO DAS PROPRIEDADES DO J/ψ
    // ============================================================
    float px_total = mu1->momentum().px() + mu2->momentum().px();
    float py_total = mu1->momentum().py() + mu2->momentum().py();
    float pz_total = mu1->momentum().pz() + mu2->momentum().pz();
    float e_total = mu1->momentum().e() + mu2->momentum().e();
    
    float jpsi_mass = std::sqrt(e_total * e_total - 
                                 px_total * px_total - 
                                 py_total * py_total - 
                                 pz_total * pz_total);
    float jpsi_pt = std::sqrt(px_total * px_total + py_total * py_total);
    float jpsi_eta = std::atanh(pz_total / std::sqrt(px_total*px_total + py_total*py_total + pz_total*pz_total));
    float jpsi_phi = std::atan2(py_total, px_total);
    
    // ============================================================
    // CRIAR SEED COMPOSTO
    // ============================================================
    float eta_avg = (eta1 + eta2) / 2.0;
    float phi_avg = (phi1 + phi2) / 2.0;
    
    auto seed = generator::Seed(eta_avg, phi_avg);
    
    // ============================================================
    // ADICIONA O PRIMEIRO MÚON (μ⁺) - PDG ID = 13
    // ============================================================
    seed.emplace_back(1, 0,                    // index=1, parent=0
                      mu1->pid(),              // PDG ID (13 para μ⁺)
                      mu1->momentum().px(),    // px
                      mu1->momentum().py(),    // py
                      mu1->momentum().pz(),    // pz
                      eta1,                    // eta
                      phi1,                    // phi
                      vtx1->position().px(),   // vx
                      vtx1->position().py(),   // vy
                      zVertexPos1,             // vz
                      vtx1->position().t() + main_event_t, // t
                      mu1->momentum().e(),     // e
                      mu1->momentum().pt());   // pt
    
    // ============================================================
    // ADICIONA O SEGUNDO MÚON (μ⁻) - PDG ID = -13
    // ============================================================
    seed.emplace_back(2, 0,                    // index=2, parent=0
                      mu2->pid(),              // PDG ID (-13 para μ⁻)
                      mu2->momentum().px(),    // px
                      mu2->momentum().py(),    // py
                      mu2->momentum().pz(),    // pz
                      eta2,                    // eta
                      phi2,                    // phi
                      vtx2->position().px(),   // vx
                      vtx2->position().py(),   // vy
                      zVertexPos2,             // vz
                      vtx2->position().t() + main_event_t, // t
                      mu2->momentum().e(),     // e
                      mu2->momentum().pt());   // pt
    
    // ============================================================
    // ADICIONAR O J/ψ COMO UMA PARTÍCULA ESPECIAL (PDG=443)
    // ============================================================
    seed.emplace_back(3, 0,                    // index=3, parent=0
                      443,                     // PDG ID do J/ψ
                      px_total,                // px total
                      py_total,                // py total
                      pz_total,                // pz total
                      jpsi_eta,                // eta do J/ψ
                      jpsi_phi,                // phi do J/ψ
                      0.0,                     // vx
                      0.0,                     // vy
                      0.0,                     // vz
                      main_event_t,            // t
                      e_total,                 // e total
                      jpsi_pt);                // pt do J/ψ
    
    // Adiciona o seed composto ao contexto
    ctx.push_back(seed);
    
    // ============================================================
    // LOG COMPLETO COM TODAS AS INFORMAÇÕES
    // ============================================================
    MSG_INFO("========================================");
    MSG_INFO("J/ψ -> μ⁺μ⁻ Pair Information:");
    MSG_INFO("");
    MSG_INFO("  Muon 1 (μ⁺):");
    MSG_INFO("    PDG ID   = " << mu1->pid());
    MSG_INFO("    pT       = " << mu1->momentum().pt() << " GeV");
    MSG_INFO("    eta      = " << eta1);
    MSG_INFO("    phi      = " << phi1);
    MSG_INFO("    px       = " << mu1->momentum().px() << " GeV");
    MSG_INFO("    py       = " << mu1->momentum().py() << " GeV");
    MSG_INFO("    pz       = " << mu1->momentum().pz() << " GeV");
    MSG_INFO("    Energy   = " << mu1->momentum().e() << " GeV");
    MSG_INFO("");
    MSG_INFO("  Muon 2 (μ⁻):");
    MSG_INFO("    PDG ID   = " << mu2->pid());
    MSG_INFO("    pT       = " << mu2->momentum().pt() << " GeV");
    MSG_INFO("    eta      = " << eta2);
    MSG_INFO("    phi      = " << phi2);
    MSG_INFO("    px       = " << mu2->momentum().px() << " GeV");
    MSG_INFO("    py       = " << mu2->momentum().py() << " GeV");
    MSG_INFO("    pz       = " << mu2->momentum().pz() << " GeV");
    MSG_INFO("    Energy   = " << mu2->momentum().e() << " GeV");
    MSG_INFO("");
    MSG_INFO("  J/ψ Properties:");
    MSG_INFO("    PDG ID   = 443");
    MSG_INFO("    Mass     = " << jpsi_mass << " GeV");
    MSG_INFO("    pT       = " << jpsi_pt << " GeV");
    MSG_INFO("    eta      = " << jpsi_eta);
    MSG_INFO("    phi      = " << jpsi_phi);
    MSG_INFO("");
    MSG_INFO("  Angular Separation:");
    MSG_INFO("    Δη       = " << delta_eta);
    MSG_INFO("    Δφ       = " << delta_phi);
    MSG_INFO("    ΔR       = " << delta_R);
    MSG_INFO("========================================");
  }

  ctx.setEventNumber(evt.event_number());
  return StatusCode::SUCCESS;
}

StatusCode Jpsi::finalize()
{
  MSG_INFO("Finalize the Jpsi Event.");
  if (generator()->finalize().isFailure()) {
    MSG_FATAL("Unable to finalize the Jpsi generator. Abort!");
  }
  return StatusCode::SUCCESS;
}
