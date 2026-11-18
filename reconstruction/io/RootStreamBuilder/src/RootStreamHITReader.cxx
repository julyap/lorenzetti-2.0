#include "RootStreamHITReader.h"

// ===== CORRIGIDO: include de MuonHitContainer movido para o .cxx =====
#include "MuonHit/MuonHitContainer.h"

#include "CaloHit/CaloHitConverter.h"
#include "MuonHit/MuonHitConverter.h"
#include "EventInfo/EventInfoConverter.h"
#include "TruthParticle/TruthParticleConverter.h"
#include "EventInfo/SeedConverter.h"

#include "GaugiKernel/EDM.h"

using namespace Gaugi;
using namespace SG;

//=====================================================================

RootStreamHITReader::RootStreamHITReader( std::string name ) :
  IMsgService(name),
  Algorithm()
{
  // --- Oficiais (PRESERVADAS) ---
  declareProperty( "InputFile"         , m_inputFile        = "" );
  declareProperty( "OutputEventKey"    , m_eventKey         = "EventInfo" );
  declareProperty( "OutputTruthKey"    , m_truthKey         = "Particles" );
  declareProperty( "OutputHitsKey"     , m_hitsKey          = "Hits" );
  declareProperty( "OutputSeedsKey"    , m_seedsKey         = "Seeds" );
  declareProperty( "NtupleName"        , m_ntupleName       = "CollectionTree" );
  declareProperty( "OutputLevel"       , m_outputLevel      = 1 );

  // --- Aditivo: alias da sua versao (se passado, tem precedencia) ---
  declareProperty( "OutputCaloHitsKey" , m_caloHitsKey      = "" );

  // --- Aditivo: muon ---
  declareProperty( "OutputMuonHitsKey" , m_muonHitsKey      = "MuonHits" );
  declareProperty( "EnableMuonInput"   , m_enableMuonInput  = true );
}

//=====================================================================

RootStreamHITReader::~RootStreamHITReader() {}

//=====================================================================

StatusCode RootStreamHITReader::initialize()
{
  CHECK_INIT();
  setMsgLevel(m_outputLevel);
  return StatusCode::SUCCESS;
}

//=====================================================================

StatusCode RootStreamHITReader::finalize()
{
  return StatusCode::SUCCESS;
}

//=====================================================================

StatusCode RootStreamHITReader::bookHistograms( EventContext &ctx ) const
{
  MSG_INFO( "Opening input file " << m_inputFile );
  auto store = ctx.getStoreGateSvc();

  TFile* file = new TFile( m_inputFile.c_str(), "READ" );
  if (!file || file->IsZombie()) {
    MSG_ERROR( "Failed to open input file: " << m_inputFile );
    return StatusCode::FAILURE;
  }

  store->decorate( "events", file );

  return StatusCode::SUCCESS;
}

//=====================================================================

StatusCode RootStreamHITReader::pre_execute( EventContext& ) const
{
  return StatusCode::SUCCESS;
}

//=====================================================================

StatusCode RootStreamHITReader::execute( EventContext&, const G4Step* ) const
{
  return StatusCode::SUCCESS;
}

//=====================================================================

StatusCode RootStreamHITReader::execute( EventContext &ctx, int evt ) const
{
  return deserialize( evt, ctx );
}

//=====================================================================

StatusCode RootStreamHITReader::post_execute( EventContext& ) const
{
  return StatusCode::SUCCESS;
}

//=====================================================================

StatusCode RootStreamHITReader::fillHistograms( EventContext& ) const
{
  return StatusCode::SUCCESS;
}

//=====================================================================

StatusCode RootStreamHITReader::deserialize( int evt, EventContext &ctx ) const
{
  std::vector<xAOD::EventInfo_t>*      ev      = nullptr;
  std::vector<xAOD::Seed_t>*           sd      = nullptr;
  std::vector<xAOD::TruthParticle_t>*  tr      = nullptr;
  std::vector<xAOD::CaloHit_t>*        caloHits= nullptr;
  std::vector<xAOD::MuonHit_t>*        muonHits= nullptr;

  auto store = ctx.getStoreGateSvc();
  TFile* file = (TFile*) store->decorator("events");

  if (!file || file->IsZombie()) {
    MSG_ERROR( "Invalid ROOT file" );
    return StatusCode::FAILURE;
  }

  TTree* tree = (TTree*) file->Get( m_ntupleName.c_str() );

  if (!tree) {
    MSG_ERROR( "Failed to get tree: " << m_ntupleName );
    return StatusCode::FAILURE;
  }


  const std::string out_calo_hits = m_caloHitsKey.empty() ? m_hitsKey : m_caloHitsKey;

  // Inicializar branches conforme produzido pelo HITMaker
  InitBranch( tree, "EventInfoContainer_EventInfo", &ev );
  InitBranch( tree, "SeedContainer_Seeds", &sd );
  InitBranch( tree, "TruthParticleContainer_Particles", &tr );
  InitBranch( tree, ("CaloHitContainer_" + out_calo_hits).c_str(), &caloHits );
  if (m_enableMuonInput) {
    InitBranch( tree, ("MuonHitContainer_" + m_muonHitsKey).c_str(), &muonHits );
  }

  if (tree->GetEntries() <= evt) {
    MSG_ERROR( "Event " << evt << " out of range. Tree has only " << tree->GetEntries() << " entries." );
    return StatusCode::FAILURE;
  }

  tree->GetEntry( evt );

  MSG_INFO( "Processing event " << evt
            << " with " << (caloHits ? caloHits->size() : 0) << " CaloHits"
            << " and " << (muonHits ? muonHits->size() : 0) << " MuonHits" );

  /* ================= EventInfo (OFICIAL) ================= */
  {
    WriteHandle<xAOD::EventInfoContainer> h( m_eventKey, ctx );
    h.record( std::make_unique<xAOD::EventInfoContainer>() );

    if (ev && !ev->empty()) {
      xAOD::EventInfo* e = nullptr;
      xAOD::EventInfoConverter cnv;
      cnv.convert( ev->at(0), e );
      if (e) h->push_back( e );
    }
  }

  /* ================= Truth (OFICIAL) ================= */
  {
    WriteHandle<xAOD::TruthParticleContainer> h( m_truthKey, ctx );
    h.record( std::make_unique<xAOD::TruthParticleContainer>() );

    if (tr && !tr->empty()) {
      xAOD::TruthParticleConverter cnv;
      for ( auto& t : *tr ) {
        xAOD::TruthParticle* p = nullptr;
        cnv.convert( t, p );
        if (p) {
          // Copiar variaveis de par (aditivo)
          p->set_pair_pdg1(t.pair_pdg1);
          p->set_pair_pdg2(t.pair_pdg2);
          p->set_pair_pt1(t.pair_pt1);
          p->set_pair_pt2(t.pair_pt2);
          p->set_pair_eta1(t.pair_eta1);
          p->set_pair_eta2(t.pair_eta2);
          p->set_pair_phi1(t.pair_phi1);
          p->set_pair_phi2(t.pair_phi2);
          p->set_pair_delta_eta(t.pair_delta_eta);
          p->set_pair_delta_phi(t.pair_delta_phi);
          p->set_pair_delta_R(t.pair_delta_R);
          p->set_pair_mass(t.pair_mass);
          p->set_pair_pt(t.pair_pt);
          p->set_pair_eta(t.pair_eta);
          p->set_pair_phi(t.pair_phi);
          p->set_pair_seedid(t.pair_seedid);
          h->push_back( p );
        }
      }
    }
  }

  /* ================= Seeds (OFICIAL) ================= */
  {
    WriteHandle<xAOD::SeedContainer> h( m_seedsKey, ctx );
    h.record( std::make_unique<xAOD::SeedContainer>() );

    if (sd && !sd->empty()) {
      xAOD::SeedConverter cnv;
      for ( auto& s : *sd ) {
        xAOD::Seed* o = nullptr;
        cnv.convert( s, o );
        if (o) h->push_back( o );
      }
    }
  }

  /* ================= CaloHits (OFICIAL) ================= */
  if ( caloHits && !caloHits->empty() ) {
    WriteHandle<xAOD::CaloHitContainer> h( out_calo_hits, ctx );
    h.record( std::make_unique<xAOD::CaloHitContainer>() );

    xAOD::CaloHitConverter cnv;
    for ( auto& caloHit_t : *caloHits ) {
      try {
        xAOD::CaloHit* caloHit = nullptr;
        if (cnv.convert( caloHit_t, caloHit )) {
          if (caloHit) h->push_back( caloHit );
        }
      } catch (const std::exception& e) {
        MSG_WARNING( "Failed to convert CaloHit: " << e.what() );
      }
    }
    MSG_INFO( "Converted " << h->size() << " CaloHits" );
  }

  /* ================= MuonHits (ADITIVO) ================= */
  if ( m_enableMuonInput && muonHits && !muonHits->empty() ) {
    WriteHandle<xAOD::MuonHitContainer> h( m_muonHitsKey, ctx );
    h.record( std::make_unique<xAOD::MuonHitContainer>() );

    xAOD::MuonHitConverter cnv;
    for ( auto& muonHit_t : *muonHits ) {
      try {
        xAOD::MuonHit* muonHit = nullptr;
        if (cnv.convert( muonHit_t, muonHit )) {
          if (muonHit) h->push_back( muonHit );
        }
      } catch (const std::exception& e) {
        MSG_WARNING( "Failed to convert MuonHit: " << e.what() );
      }
    }
    MSG_INFO( "Converted " << h->size() << " MuonHits" );
  }

  return StatusCode::SUCCESS;
}

//=====================================================================

template <class T>
void RootStreamHITReader::InitBranch( TTree* tree, std::string name, T* ptr ) const
{
  if ( !tree->FindBranch( name.c_str() ) ) {
    MSG_WARNING( "Branch not found: " << name );
    *ptr = nullptr;
    return;
  }
  tree->SetBranchStatus ( name.c_str(), 1 );
  tree->SetBranchAddress( name.c_str(), ptr );
}
