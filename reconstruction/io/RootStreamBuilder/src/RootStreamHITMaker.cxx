#include "RootStreamHITMaker.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <set>
#include <tuple>

#include "CaloHit/CaloHitConverter.h"
#include "MuonHit/MuonHitConverter.h"
#include "EventInfo/EventInfoConverter.h"
#include "TruthParticle/TruthParticleConverter.h"
#include "EventInfo/SeedConverter.h"
#include "GaugiKernel/EDM.h"
#include "G4Kernel/CaloPhiRange.h"

using namespace SG;

// =====================================================================
// Funções auxiliares de identificação de partículas (aditivas)
// =====================================================================
bool Gaugi::RootStreamHITMaker::isElectron(int pdgId) {
  return std::abs(pdgId) == 11;
}

bool Gaugi::RootStreamHITMaker::isMuon(int pdgId) {
  return std::abs(pdgId) == 13;
}

bool Gaugi::RootStreamHITMaker::isPhoton(int pdgId) {
  return std::abs(pdgId) == 22;
}

bool Gaugi::RootStreamHITMaker::isHadron(int pdgId) {
  int absPdgId = std::abs(pdgId);
  return (absPdgId > 100 && absPdgId < 1000) || (absPdgId > 1000 && absPdgId < 10000);
}

bool Gaugi::RootStreamHITMaker::isHitInRoI(float hit_eta, float hit_phi, 
                                    const std::vector<float>& particle_etas,
                                    const std::vector<float>& particle_phis,
                                    float etaWindow, float phiWindow) const {
  if (particle_etas.empty()) return false;
  for (size_t i = 0; i < particle_etas.size(); ++i) {
    float dEta = std::abs(particle_etas[i] - hit_eta);
    float dPhi = std::abs(CaloPhiRange::diff(particle_phis[i], hit_phi));
    if (dEta < etaWindow/2 && dPhi < phiWindow/2) return true;
  }
  return false;
}

// =====================================================================
// Construtor (oficial + muon aditivo)
// =====================================================================
Gaugi::RootStreamHITMaker::RootStreamHITMaker(std::string name) : 
  IMsgService(name),
  Algorithm()
{
  // --- Oficiais (PRESERVADAS) ---
  declareProperty( "InputEventKey"           , m_inputEventKey="Events"                );
  declareProperty( "InputTruthKey"           , m_inputTruthKey="Particles"             );
  declareProperty( "InputHitsKey"            , m_inputHitsKey="Hits"                   );
  declareProperty( "InputSeedsKey"           , m_inputSeedsKey="Seeds"                 );
  declareProperty( "OutputEventKey"          , m_outputEventKey="EventInfo"            );
  declareProperty( "OutputTruthKey"          , m_outputTruthKey="Particles"            );
  declareProperty( "OutputHitsKey"           , m_outputHitsKey="Hits"                  );
  declareProperty( "OutputSeedsKey"          , m_outputSeedsKey="Seeds"                );
  declareProperty( "OutputLevel"             , m_outputLevel=1                         );
  declareProperty( "NtupleName"              , m_ntupleName="CollectionTree"           );
  declareProperty( "OnlyRoI"                 , m_onlyRoI=false                         );
  declareProperty( "EtaWindow"               , m_etaWindow=0.6                         );
  declareProperty( "PhiWindow"               , m_phiWindow=0.6                         );
  declareProperty( "KeepCells"               , m_cellHashes={}                         );

  // --- Aditivo: aliases da sua versao ---
  declareProperty( "InputCaloHitsKey"        , m_inputCaloHitsKey=""                   );
  declareProperty( "OutputCaloHitsKey"       , m_outputCaloHitsKey=""                  );

  // --- Aditivo: muon ---
  declareProperty( "InputMuonHitsKey"        , m_inputMuonHitsKey="MuonHits"           );
  declareProperty( "OutputMuonHitsKey"       , m_outputMuonHitsKey="MuonHits"          );
  declareProperty( "MuonEtaWindow"           , m_muonEtaWindow=0.6                     );
  declareProperty( "MuonPhiWindow"           , m_muonPhiWindow=0.6                     );
  declareProperty( "EnableMuonOutput"        , m_enableMuonOutput=true                 );
}

//!=====================================================================

Gaugi::RootStreamHITMaker::~RootStreamHITMaker()
{;}

//!=====================================================================

StatusCode Gaugi::RootStreamHITMaker::initialize()
{
  CHECK_INIT();
  setMsgLevel(m_outputLevel);
  MSG_INFO("Input Event Key: " << m_inputEventKey);
  MSG_INFO("Input Truth Key: " << m_inputTruthKey);
  MSG_INFO("Input Hits Key: " << m_inputHitsKey);
  MSG_INFO("Input Seeds Key: " << m_inputSeedsKey);
  MSG_INFO("Output Event Key: " << m_outputEventKey);
  MSG_INFO("Output Truth Key: " << m_outputTruthKey);
  MSG_INFO("Output Hits Key: " << m_outputHitsKey);
  MSG_INFO("Output Seeds Key: " << m_outputSeedsKey);
  MSG_INFO("Output Level: " << m_outputLevel);
  MSG_INFO("Ntuple Name: " << m_ntupleName);
  MSG_INFO("Only RoI: " << (m_onlyRoI ? "true" : "false"));
  MSG_INFO("Eta Window: " << m_etaWindow);
  MSG_INFO("Phi Window: " << m_phiWindow);
  MSG_INFO("Keep Cells: " << (m_cellHashes.empty() ? "false" : "true"));
  MSG_INFO("Muon: " << (m_enableMuonOutput ? "ENABLED" : "DISABLED"));
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode Gaugi::RootStreamHITMaker::bookHistograms(SG::EventContext &ctx) const
{
  auto store = ctx.getStoreGateSvc();

  // --- Containers oficiais ---
  std::vector<xAOD::CaloHit_t> *container_calo_hits = new std::vector<xAOD::CaloHit_t>();
  std::vector<xAOD::EventInfo_t> *container_event = new std::vector<xAOD::EventInfo_t>();
  std::vector<xAOD::TruthParticle_t> *container_truth = new std::vector<xAOD::TruthParticle_t>();
  std::vector<xAOD::Seed_t> *container_seeds = new std::vector<xAOD::Seed_t>();

  // --- Container aditivo de muon ---
  std::vector<xAOD::MuonHit_t> *container_muon_hits = new std::vector<xAOD::MuonHit_t>();

  store->cd();
  TTree *tree = new TTree(m_ntupleName.c_str(), "");
  
  // --- Branches oficiais ---
  tree->Branch(("EventInfoContainer_" + m_outputEventKey).c_str(), &container_event);
  tree->Branch(("SeedContainer_" + m_outputSeedsKey).c_str(), &container_seeds);
  tree->Branch(("TruthParticleContainer_" + m_outputTruthKey).c_str(), &container_truth);

  // CaloHits: usa o alias se passado, senao o nome oficial
  const std::string out_calo_hits = m_outputCaloHitsKey.empty() ? m_outputHitsKey : m_outputCaloHitsKey;
  tree->Branch(("CaloHitContainer_" + out_calo_hits).c_str(), &container_calo_hits);

  // --- Aditivo: muon ---
  if (m_enableMuonOutput) {
    tree->Branch(("MuonHitContainer_" + m_outputMuonHitsKey).c_str(), &container_muon_hits);
  }

  store->add(tree);
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode Gaugi::RootStreamHITMaker::pre_execute(SG::EventContext &/*ctx*/) const { return StatusCode::SUCCESS; }
StatusCode Gaugi::RootStreamHITMaker::execute(SG::EventContext &/*ctx*/, const G4Step* /*step*/) const { return StatusCode::SUCCESS; }
StatusCode Gaugi::RootStreamHITMaker::execute(SG::EventContext &/*ctx*/, int /*evt*/) const { return StatusCode::SUCCESS; }
StatusCode Gaugi::RootStreamHITMaker::finalize() { return StatusCode::SUCCESS; }
StatusCode Gaugi::RootStreamHITMaker::post_execute(SG::EventContext &/*ctx*/) const { return StatusCode::SUCCESS; }
StatusCode Gaugi::RootStreamHITMaker::fillHistograms(SG::EventContext &ctx) const { return serialize(ctx); }

//!=====================================================================

template <class T>
void Gaugi::RootStreamHITMaker::InitBranch(TTree* fChain, std::string branch_name, T* param) const
{
  std::string bname = branch_name;
  if (fChain->GetAlias(bname.c_str())) bname = std::string(fChain->GetAlias(bname.c_str()));
  if (!fChain->FindBranch(bname.c_str())) {
    MSG_WARNING("unknown branch " << bname);
    return;
  }
  fChain->SetBranchStatus(bname.c_str(), 1.);
  fChain->SetBranchAddress(bname.c_str(), param);
}

//!=====================================================================

StatusCode Gaugi::RootStreamHITMaker::serialize(SG::EventContext &ctx) const
{
  auto store = ctx.getStoreGateSvc();
  store->cd();
  TTree *tree = store->tree(m_ntupleName);

  // --- Ponteiros oficiais ---
  std::vector<xAOD::CaloHit_t> *container_calo_hits = nullptr;
  std::vector<xAOD::EventInfo_t> *container_event = nullptr;
  std::vector<xAOD::Seed_t> *container_seeds = nullptr;
  std::vector<xAOD::TruthParticle_t> *container_truth = nullptr;

  // --- Ponteiros aditivos de muon ---
  std::vector<xAOD::MuonHit_t> *container_muon_hits = nullptr;

  MSG_DEBUG("Link all branches...");

  const std::string in_calo_hits  = m_inputCaloHitsKey.empty()  ? m_inputHitsKey  : m_inputCaloHitsKey;
  const std::string out_calo_hits = m_outputCaloHitsKey.empty() ? m_outputHitsKey : m_outputCaloHitsKey;

  // --- Branches oficiais ---
  InitBranch(tree, ("EventInfoContainer_" + m_outputEventKey).c_str(), &container_event);
  InitBranch(tree, ("SeedContainer_" + m_outputSeedsKey).c_str(), &container_seeds);
  InitBranch(tree, ("TruthParticleContainer_" + m_outputTruthKey).c_str(), &container_truth);
  InitBranch(tree, ("CaloHitContainer_" + out_calo_hits).c_str(), &container_calo_hits);

  // --- Branch aditiva de muon ---
  if (m_enableMuonOutput) {
    InitBranch(tree, ("MuonHitContainer_" + m_outputMuonHitsKey).c_str(), &container_muon_hits);
  }

  // --- EventInfo (OFICIAL) ---
  {
    MSG_DEBUG("Serialize EventInfo...");
    SG::ReadHandle<xAOD::EventInfoContainer> event(m_inputEventKey, ctx);

    if (!event.isValid()) {
      MSG_FATAL("It's not possible to read the xAOD::EventInfoContainer from this Context");
    }

    xAOD::EventInfo_t event_t;
    xAOD::EventInfoConverter cnv;
    cnv.convert((**event.ptr()).front(), event_t);
    container_event->push_back(event_t);
  }

  // --- Seeds (OFICIAL) ---
  {
    MSG_DEBUG("Serialize Seed..");
    SG::ReadHandle<xAOD::SeedContainer> container(m_inputSeedsKey, ctx);
    if (!container.isValid()) {
      MSG_FATAL("It's not possible to read the xAOD::SeedContainer from this Context using this key " << m_inputSeedsKey);
    }
    for (const auto seed : **container.ptr()) {
      xAOD::Seed_t seed_t;
      xAOD::SeedConverter cnv;
      cnv.convert(seed, seed_t);
      container_seeds->push_back(seed_t);
    }
  }

  // --- Truth Particles + cálculo de par (ADITIVO) ---
  bool hasCaloParticles = false;
  bool hasMuons = false;
  std::vector<float> muon_etas, muon_phis, calo_etas, calo_phis;
  
  {
    SG::ReadHandle<xAOD::TruthParticleContainer> container(m_inputTruthKey, ctx);
    if (container.isValid()) {
      for (const auto par : **container.ptr()) {
        if (!par) continue;
        int pdgId = par->pdgid();
        
        if (isMuon(pdgId)) {
          hasMuons = true;
          muon_etas.push_back(par->eta());
          muon_phis.push_back(par->phi());
        } else if (isElectron(pdgId) || isPhoton(pdgId) || isHadron(pdgId)) {
          hasCaloParticles = true;
          calo_etas.push_back(par->eta());
          calo_phis.push_back(par->phi());
        }
        
        xAOD::TruthParticle_t par_t;
        xAOD::TruthParticleConverter cnv;
        cnv.convert(par, par_t);
        container_truth->push_back(par_t);
      }
    }
  }

  // Cálculo das variáveis de par (ADITIVO)
  if (container_truth) {
    std::map<int, std::vector<int>> seed_particles;
    for (size_t i = 0; i < container_truth->size(); ++i) {
      int seed = (*container_truth)[i].seedid;
      seed_particles[seed].push_back(i);
    }
    
    for (const auto& seed_pair : seed_particles) {
      int seed_id = seed_pair.first;
      const std::vector<int>& indices = seed_pair.second;
      
      if (indices.size() >= 2) {
        int idx1 = indices[0];
        int idx2 = indices[1];
        auto& p1 = (*container_truth)[idx1];
        auto& p2 = (*container_truth)[idx2];
        
        float delta_eta = p1.eta - p2.eta;
        float delta_phi = p1.phi - p2.phi;
        while (delta_phi > M_PI) delta_phi -= 2 * M_PI;
        while (delta_phi < -M_PI) delta_phi += 2 * M_PI;
        float delta_R = std::sqrt(delta_eta * delta_eta + delta_phi * delta_phi);
        
        float px_total = p1.px + p2.px;
        float py_total = p1.py + p2.py;
        float pz_total = p1.pz + p2.pz;
        float E_total = p1.e + p2.e;
        
        float mass = std::sqrt(std::max(0.0f, E_total * E_total - 
                                        px_total * px_total - 
                                        py_total * py_total - 
                                        pz_total * pz_total));
        float pt_pair = std::sqrt(px_total * px_total + py_total * py_total);
        float eta_pair = std::atanh(pz_total / std::sqrt(px_total*px_total + py_total*py_total + pz_total*pz_total));
        float phi_pair = std::atan2(py_total, px_total);
        
        // Salvar no p1
        p1.pair_pdg1 = p1.pdgid;   p1.pair_pdg2 = p2.pdgid;
        p1.pair_pt1 = p1.et;       p1.pair_pt2 = p2.et;
        p1.pair_eta1 = p1.eta;     p1.pair_eta2 = p2.eta;
        p1.pair_phi1 = p1.phi;     p1.pair_phi2 = p2.phi;
        p1.pair_delta_eta = delta_eta; p1.pair_delta_phi = delta_phi;
        p1.pair_delta_R = delta_R; p1.pair_mass = mass;
        p1.pair_pt = pt_pair;      p1.pair_eta = eta_pair;
        p1.pair_phi = phi_pair;    p1.pair_seedid = seed_id;
        
        // Salvar no p2
        p2.pair_pdg1 = p1.pdgid;   p2.pair_pdg2 = p2.pdgid;
        p2.pair_pt1 = p1.et;       p2.pair_pt2 = p2.et;
        p2.pair_eta1 = p1.eta;     p2.pair_eta2 = p2.eta;
        p2.pair_phi1 = p1.phi;     p2.pair_phi2 = p2.phi;
        p2.pair_delta_eta = delta_eta; p2.pair_delta_phi = delta_phi;
        p2.pair_delta_R = delta_R; p2.pair_mass = mass;
        p2.pair_pt = pt_pair;      p2.pair_eta = eta_pair;
        p2.pair_phi = phi_pair;    p2.pair_seedid = seed_id;
      }
    }
  }

  // --- CaloHits (OFICIAL) ---
  {
    MSG_DEBUG("Serialize CaloHits...");
    bool keepCells = !m_cellHashes.empty();
    MSG_INFO("Keep Cells? " << (keepCells ? "Yes" : "No"));

    SG::ReadHandle<xAOD::TruthParticleContainer> particles(m_inputTruthKey, ctx);
    SG::ReadHandle<xAOD::CaloHitContainer> container(in_calo_hits, ctx);
    if (!container.isValid()) {
      MSG_FATAL("It's not possible to read the xAOD::CaloHitContainer from this Contaxt using this key " << in_calo_hits);
    }

    float etot = 0;
    for (const auto hit : **container.ptr()) {
      if (m_onlyRoI) {
        bool match = false;
        for (const auto& par : **particles.ptr()) {
          float deltaEta = std::abs(par->eta() - hit->eta());
          float deltaPhi = std::abs(CaloPhiRange::diff(par->phi(), hit->phi()));
          if (deltaEta < m_etaWindow/2 && deltaPhi < m_phiWindow/2) {
            match = true;
          } else if (keepCells) {
            if (std::find(m_cellHashes.begin(), m_cellHashes.end(), hit->hash()) != m_cellHashes.end()) {
              match = true;
            }
          }
          if (match) break;
        }
        if (!match) continue;
      }

      xAOD::CaloHit_t hit_t;
      xAOD::CaloHitConverter hit_cnv;
      hit_cnv.convert(hit, hit_t);

      if (hit->hash() != hit_t.hash) {
        MSG_FATAL("Hit missmatch " << hit->hash() << " != " << hit_t.hash);
      }
      
      container_calo_hits->push_back(hit_t);
      etot += hit->edep();
    }
    MSG_DEBUG("Container hit size is " << container_calo_hits->size() << " and total energy " << etot << " MeV");
  }

  // --- MuonHits (ADITIVO) ---
  if (m_enableMuonOutput && container_muon_hits) {
    SG::ReadHandle<xAOD::MuonHitContainer> muon_container(m_inputMuonHitsKey, ctx);
    
    if (muon_container.isValid()) {
      for (const auto hit : **muon_container.ptr()) {
        if (!hit) continue;
        
        try {
          if (m_onlyRoI && (m_muonEtaWindow > 0 || m_muonPhiWindow > 0) && hasMuons) {
            float hit_eta = -std::log(std::tan(std::acos(hit->z() / std::sqrt(hit->x()*hit->x() + hit->y()*hit->y() + hit->z()*hit->z()))/2.0));
            float hit_phi = std::atan2(hit->y(), hit->x());
            if (!isHitInRoI(hit_eta, hit_phi, muon_etas, muon_phis, m_muonEtaWindow, m_muonPhiWindow))
              continue;
          }
          
          xAOD::MuonHit_t hit_t;
          xAOD::MuonHitConverter cnv;
          if (cnv.convert(hit, hit_t)) {
            container_muon_hits->push_back(hit_t);
          }
        } catch (const std::exception& e) {
          MSG_ERROR("Erro processando MuonHit: " << e.what());
        }
      }
    } else {
      MSG_WARNING("MuonHitContainer NAO encontrado com a chave: " << m_inputMuonHitsKey);
    }
  }

  tree->Fill();
  return StatusCode::SUCCESS;
}
