#include "RootStreamHITReader.h"

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
  declareProperty( "InputFile"         , m_inputFile        = "" );
  declareProperty( "OutputEventKey"    , m_eventKey         = "EventInfo" );
  declareProperty( "OutputTruthKey"    , m_truthKey         = "Particles" );
  declareProperty( "OutputCaloHitsKey" , m_caloHitsKey      = "CaloHits" );
  declareProperty( "OutputMuonHitsKey" , m_muonHitsKey      = "MuonHits" );
  declareProperty( "OutputSeedsKey"    , m_seedsKey         = "Seeds" );
  declareProperty( "NtupleName"        , m_ntupleName       = "CollectionTree" );
  declareProperty( "OutputLevel"       , m_outputLevel      = 1 );
  declareProperty( "EnableMuonInput"   , m_enableMuonInput  = true );
}

//=====================================================================

RootStreamHITReader::~RootStreamHITReader() {}

//=====================================================================

StatusCode RootStreamHITReader::initialize()
{
  CHECK_INIT();
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

StatusCode
RootStreamHITReader::deserialize( int evt, EventContext &ctx ) const
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

  // Inicializar branches conforme gravados no arquivo HIT

   InitBranch( tree, "EventInfoContainer_Events", &ev );
   InitBranch( tree, "SeedContainer_Seeds", &sd );
   InitBranch( tree, "TruthParticleContainer_Particles", &tr );
   InitBranch( tree, "CaloHitContainer_Hits", &caloHits );
   InitBranch( tree, "MuonHitContainer_MuonHits", &muonHits );
  if (tree->GetEntries() <= evt) {
    MSG_ERROR( "Event " << evt << " out of range. Tree has only " << tree->GetEntries() << " entries." );
    return StatusCode::FAILURE;
  }

  tree->GetEntry( evt );

  MSG_INFO( "Processing event " << evt 
            << " with " << (caloHits ? caloHits->size() : 0) << " CaloHits"
            << " and " << (muonHits ? muonHits->size() : 0) << " MuonHits" );

  /* ================= EventInfo ================= */
  {
    WriteHandle<xAOD::EventInfoContainer> h( m_eventKey, ctx );
    h.record( std::make_unique<xAOD::EventInfoContainer>() );

    if (ev && !ev->empty()) {
      xAOD::EventInfo* e = nullptr;
      xAOD::EventInfoConverter cnv;
      cnv.convert( ev->at(0), e );
      if (e) {
        h->push_back( e );
      } else {
        MSG_WARNING( "Failed to convert EventInfo" );
      }
    } else {
      MSG_WARNING( "No EventInfo found in ROOT file" );
    }
  }

  /* ================= Truth - COM AS NOVAS VARIÁVEIS ================= */
  {
    WriteHandle<xAOD::TruthParticleContainer> h( m_truthKey, ctx );
    h.record( std::make_unique<xAOD::TruthParticleContainer>() );

    if (tr && !tr->empty()) {
      xAOD::TruthParticleConverter cnv;
      for ( auto& t : *tr ) {
        xAOD::TruthParticle* p = nullptr;
        cnv.convert( t, p );
        if (p) {
          // ============================================================
          // COPIAR AS NOVAS VARIÁVEIS DO TruthParticle_t PARA O TruthParticle
          // ============================================================
          // O TruthParticleConverter não copia as novas variáveis,
          // então copiamos manualmente usando os setters.
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
          
          MSG_DEBUG( "Particle: pdg=" << p->pdgid() 
                     << ", pair_mass=" << p->pair_mass() 
                     << ", pair_delta_R=" << p->pair_delta_R() );
          h->push_back( p );
        }
      }
      MSG_INFO( "Converted " << h->size() << " TruthParticles with pair variables" );
    } else {
      MSG_INFO( "No TruthParticles found in event " << evt );
    }
  }

  /* ================= Seeds ================= */
  {
    WriteHandle<xAOD::SeedContainer> h( m_seedsKey, ctx );
    h.record( std::make_unique<xAOD::SeedContainer>() );

    if (sd && !sd->empty()) {
      xAOD::SeedConverter cnv;
      for ( auto& s : *sd ) {
        xAOD::Seed* o = nullptr;
        cnv.convert( s, o );
        if (o) {
          h->push_back( o );
        }
      }
    }
  }

  /* ================= CaloHits ================= */
  if ( caloHits && !caloHits->empty() ) {
    int caloHitCount = 0;
    
    // Criar container para CaloHits
    WriteHandle<xAOD::CaloHitContainer> caloHits_h( m_caloHitsKey, ctx );
    caloHits_h.record( std::make_unique<xAOD::CaloHitContainer>() );
    
    xAOD::CaloHitConverter caloCnv;

    for ( auto& caloHit_t : *caloHits ) {
      try {
        xAOD::CaloHit* caloHit = nullptr;
        if (caloCnv.convert( caloHit_t, caloHit )) {
          if (caloHit) {
            caloHits_h->push_back( caloHit );
            caloHitCount++;
          }
        } else {
          MSG_WARNING( "Failed to convert CaloHit (converter returned false)" );
        }
      } catch (const std::exception& e) {
        MSG_WARNING( "Failed to convert CaloHit: " << e.what() );
      }
    }
    
    MSG_INFO( "Converted " << caloHitCount << " CaloHits" );
  } else {
    MSG_INFO( "No CaloHits found in event " << evt );
  }

  /* ================= MuonHits ================= */
  if ( m_enableMuonInput && muonHits && !muonHits->empty() ) {
    int muonHitCount = 0;
    
    // Criar container para MuonHits
    WriteHandle<xAOD::MuonHitContainer> muonHits_h( m_muonHitsKey, ctx );
    muonHits_h.record( std::make_unique<xAOD::MuonHitContainer>() );
    
    xAOD::MuonHitConverter muonCnv;

    for ( auto& muonHit_t : *muonHits ) {
      try {
        xAOD::MuonHit* muonHit = nullptr;
        if (muonCnv.convert( muonHit_t, muonHit )) {
          if (muonHit) {
            muonHits_h->push_back( muonHit );
            muonHitCount++;
          }
        } else {
          MSG_WARNING( "Failed to convert MuonHit (converter returned false)" );
        }
      } catch (const std::exception& e) {
        MSG_WARNING( "Failed to convert MuonHit: " << e.what() );
      }
    }
    
    MSG_INFO( "Converted " << muonHitCount << " MuonHits" );
  } else if ( m_enableMuonInput && (!muonHits || muonHits->empty()) ) {
    MSG_INFO( "No MuonHits found in event " << evt );
  }

  return StatusCode::SUCCESS;
}

//=====================================================================

template <class T>
void RootStreamHITReader::InitBranch( TTree* tree, std::string name, T* ptr ) const
{
  if ( !tree->FindBranch( name.c_str() ) ) {
    MSG_WARNING( "Branch not found: " << name );
    return;
  }
  tree->SetBranchStatus ( name.c_str(), 1 );
  tree->SetBranchAddress( name.c_str(), ptr );
}
