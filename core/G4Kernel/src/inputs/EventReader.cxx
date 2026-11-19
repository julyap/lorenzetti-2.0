#include "EventInfo/EventInfoContainer.h"
#include "EventInfo/SeedContainer.h"
#include "TruthParticle/TruthParticleContainer.h"
#include "G4Kernel/inputs/EventReader.h"
#include "G4Kernel/RunSequence.h"

#include "G4LorentzVector.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4TransportationManager.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include <math.h>
#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"

using namespace Gaugi;
using namespace generator;

EventReader::EventReader(std::string name):
    IMsgService(name),
    PrimaryGenerator(),
    m_evt(0),
    m_f(nullptr),
    m_ttree(nullptr),
    m_avgmu(0.0f),
    m_totmu(0.0f),
    m_runNumber(0),
    m_eventNumber(-1),
    m_bc_duration(25*ns),
    m_p_isMain(nullptr),
    m_p_pdg_id(nullptr),
    m_p_bc_id(nullptr),
    m_p_seed_id(nullptr),
    m_p_px(nullptr),
    m_p_py(nullptr),
    m_p_pz(nullptr),
    m_p_prod_x(nullptr),
    m_p_prod_y(nullptr),
    m_p_prod_z(nullptr),
    m_p_prod_t(nullptr),
    m_p_eta(nullptr),
    m_p_phi(nullptr),
    m_p_e(nullptr),
    m_p_et(nullptr)
{
  ROOT::EnableThreadSafety();  
  
  declareProperty( "InputFileName"      , m_filename=""          );
  declareProperty( "OutputEventKey"     , m_eventKey="EventInfo" );
  declareProperty( "OutputTruthKey"     , m_truthKey="Particles" );
  declareProperty( "OutputSeedKey"      , m_seedKey="Seed"       );
  declareProperty( "BunchDuration"      , m_bc_duration=25*ns    );
}

PrimaryGenerator* EventReader::copy()
{
  auto *gun = new EventReader(getLogName());
  gun->setProperty( "InputFileName" , m_filename   );
  gun->setProperty( "OutputEventKey", m_eventKey   );
  gun->setProperty( "OutputTruthKey", m_truthKey   );
  gun->setProperty( "OutputSeedKey" , m_seedKey    );
  gun->setProperty( "BunchDuration" , m_bc_duration);
  return gun;
}

EventReader::~EventReader()
{
  if( m_f ) delete m_f;
  // Não chama release() aqui: se finalize() já chamou, os ponteiros são lixo.
  // Se allocate() foi chamado mas finalize() não, há vazamento, mas sem corrupção.
}

StatusCode EventReader::initialize()
{    
  m_evt=0;
  if(m_filename != ""){
    MSG_INFO( "Open the root file: " << m_filename );
    m_f = new TFile( m_filename.c_str() , "read" );
    m_ttree = (TTree*)m_f->Get("particles");
    if( !m_ttree ){
      MSG_FATAL("Could not find TTree 'particles' in file " << m_filename);
      return StatusCode::FAILURE;
    }
    link( m_ttree );
    allocate();
  }
  return StatusCode::SUCCESS;
}

StatusCode EventReader::finalize()
{
  if(m_filename!=""){
    release();
    if( m_f ) m_f->Close();
  }
  return StatusCode::SUCCESS;
}

template <class T> 
void EventReader::InitBranch(TTree* fChain, std::string branch_name, T* param, bool message)
{
  std::string bname = branch_name;
  if (fChain->GetAlias(bname.c_str()))
     bname = std::string(fChain->GetAlias(bname.c_str()));

  if (!fChain->FindBranch(bname.c_str()) && message) {
    MSG_WARNING( "unknown branch " << bname );
    return;  
  }
  fChain->SetBranchStatus(bname.c_str(), 1.);
  fChain->SetBranchAddress(bname.c_str(), param);
}

void EventReader::link(TTree *t)
{  
  InitBranch( t, "RunNumber"  ,&m_runNumber   );
  InitBranch( t, "EventNumber",&m_eventNumber );
  InitBranch( t, "avg_mu"     ,&m_avgmu       );
  InitBranch( t, "tot_mu"     ,&m_totmu       );
  InitBranch( t, "p_isMain"   ,&m_p_isMain    );
  InitBranch( t, "p_pdg_id"   ,&m_p_pdg_id    );
  InitBranch( t, "p_bc_id"    ,&m_p_bc_id     );  
  InitBranch( t, "p_seed_id"  ,&m_p_seed_id   );
  InitBranch( t, "p_px"       ,&m_p_px        );
  InitBranch( t, "p_py"       ,&m_p_py        );
  InitBranch( t, "p_pz"       ,&m_p_pz        );
  InitBranch( t, "p_prod_x"   ,&m_p_prod_x    );
  InitBranch( t, "p_prod_y"   ,&m_p_prod_y    );
  InitBranch( t, "p_prod_z"   ,&m_p_prod_z    );
  InitBranch( t, "p_prod_t"   ,&m_p_prod_t    );
  InitBranch( t, "p_eta"      ,&m_p_eta       );
  InitBranch( t, "p_phi"      ,&m_p_phi       );
  InitBranch( t, "p_e"        ,&m_p_e         );
  InitBranch( t, "p_et"       ,&m_p_et        );
}

void EventReader::clear()
{ 
  m_avgmu       = 0.0;
  m_totmu       = 0.0;
  m_eventNumber = -1;
  m_p_isMain  ->clear();
  m_p_pdg_id  ->clear();
  m_p_bc_id   ->clear();
  m_p_seed_id ->clear();
  m_p_px      ->clear();
  m_p_py      ->clear();
  m_p_pz      ->clear();
  m_p_prod_x  ->clear();
  m_p_prod_y  ->clear();
  m_p_prod_z  ->clear();
  m_p_prod_t  ->clear();
  m_p_eta     ->clear();
  m_p_phi     ->clear();
  m_p_e       ->clear();
  m_p_et      ->clear();
}

void EventReader::allocate()
{
  m_p_isMain  = new std::vector<int>();
  m_p_pdg_id  = new std::vector<int>();
  m_p_bc_id   = new std::vector<int>();
  m_p_seed_id = new std::vector<int>();
  m_p_px      = new std::vector<float>();
  m_p_py      = new std::vector<float>();
  m_p_pz      = new std::vector<float>();
  m_p_prod_x  = new std::vector<float>();
  m_p_prod_y  = new std::vector<float>();
  m_p_prod_z  = new std::vector<float>();
  m_p_prod_t  = new std::vector<float>();
  m_p_eta     = new std::vector<float>();
  m_p_phi     = new std::vector<float>();
  m_p_e       = new std::vector<float>();
  m_p_et      = new std::vector<float>();
}

void EventReader::release()
{
  delete m_p_isMain   ; m_p_isMain   = nullptr;
  delete m_p_pdg_id   ; m_p_pdg_id   = nullptr;
  delete m_p_bc_id    ; m_p_bc_id    = nullptr;
  delete m_p_seed_id  ; m_p_seed_id  = nullptr;
  delete m_p_px       ; m_p_px       = nullptr;
  delete m_p_py       ; m_p_py       = nullptr;
  delete m_p_pz       ; m_p_pz       = nullptr;
  delete m_p_prod_x   ; m_p_prod_x   = nullptr;
  delete m_p_prod_y   ; m_p_prod_y   = nullptr;
  delete m_p_prod_z   ; m_p_prod_z   = nullptr;
  delete m_p_prod_t   ; m_p_prod_t   = nullptr;
  delete m_p_eta      ; m_p_eta      = nullptr;
  delete m_p_phi      ; m_p_phi      = nullptr;
  delete m_p_e        ; m_p_e        = nullptr;
  delete m_p_et       ; m_p_et       = nullptr;
}

// Call by geant
void EventReader::GeneratePrimaryVertex( G4Event* anEvent )
{
  clear();
  m_evt = anEvent->GetEventID();
  
  RunSequence *reco = static_cast<RunSequence*> (G4RunManager::GetRunManager()->GetNonConstCurrentRun());

  if ( m_filename != "" && m_ttree && m_evt < (unsigned int)m_ttree->GetEntries() ){
    m_ttree->GetEntry(m_evt);
    MSG_INFO( "Get event (EventReader) with number " << m_evt << " and EventNumber " << m_eventNumber);

    {
      SG::WriteHandle<xAOD::EventInfoContainer>  event(m_eventKey, reco->getContext());
      event.record( std::unique_ptr<xAOD::EventInfoContainer>( new xAOD::EventInfoContainer() ) );
      xAOD::EventInfo *evt = new xAOD::EventInfo();
      evt->setRunNumber( m_runNumber );
      evt->setEventNumber( m_eventNumber );
      evt->setAvgmu( m_avgmu );
      evt->setTotmu( m_totmu );
      event->push_back(evt);
    }

    int num_of_seeds = Load( anEvent );
    MSG_INFO( "RunNumber        : " << m_runNumber  );
    MSG_INFO( "EventNumber      : " << m_eventNumber);
    MSG_INFO( "Avgmu            : " << m_avgmu      );
    MSG_INFO( "Total Mu         : " << m_totmu );
    MSG_INFO( "Number of seeds  : " << num_of_seeds );

  }else{
    MSG_INFO( "EventReader: no generated particles. run terminated..." );
    G4RunManager::GetRunManager()->AbortRun();
  }
}

bool EventReader::CheckVertexInsideWorld(const G4ThreeVector& pos) const
{
  G4Navigator* navigator= G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
  G4VPhysicalVolume* world= navigator->GetWorldVolume();
  G4VSolid* solid= world-> GetLogicalVolume()->GetSolid();
  EInside qinside= solid->Inside(pos);
  if( qinside != kInside) return false;
  else return true;
}

int EventReader::Load( G4Event* g4event )
{
  RunSequence *reco = static_cast<RunSequence*> (G4RunManager::GetRunManager()->GetNonConstCurrentRun());

  SG::WriteHandle<xAOD::TruthParticleContainer>  particles(m_truthKey, reco->getContext());
  particles.record( std::unique_ptr<xAOD::TruthParticleContainer>( new xAOD::TruthParticleContainer() ) );

  SG::WriteHandle<xAOD::SeedContainer>  seeds(m_seedKey, reco->getContext());
  seeds.record( std::unique_ptr<xAOD::SeedContainer>( new xAOD::SeedContainer() ) );
 
  int num_of_seeds = 0;
  int seed_id = -1;

  for ( unsigned int i=0; i < m_p_e->size(); ++i )
  {
    int bc_id = m_p_bc_id->at(i);

    // --- Lê os valores para variáveis LOCAIS (NÃO escreve nos vetores do ROOT) ---
    float eta = m_p_eta->at(i);
    float px  = m_p_px->at(i);
    float py  = m_p_py->at(i);
    float pz  = m_p_pz->at(i);
    float e   = m_p_e->at(i);
    float pt  = m_p_et->at(i);

    // Corrige o sinal APENAS nas variáveis locais
    if (eta < 0 && pz > 0) {
      MSG_WARNING("CORRIGINDO SINAL: eta=" << eta << ", pz " << pz << " -> " << -pz);
      pz = -pz;
      px = -px;
      py = -py;
      // NUNCA escreva de volta em (*m_p_pz)[i], (*m_p_px)[i], (*m_p_py)[i]!
    }

    if(m_p_pdg_id->at(i)==0){ // is seed?
      seed_id++;
      xAOD::Seed *seed = new xAOD::Seed( seed_id,
                                         e*MeV, 
                                         pt*MeV, 
                                         eta, 
                                         m_p_phi->at(i)
                                       );

      MSG_INFO( "Seed " << seed_id << " in eta = " << seed->eta() << ", phi = " << seed->phi());
      seeds->push_back(seed);
      num_of_seeds++;
      continue; 
    }

    // Passe os valores locais corrigidos para o Add()
    if( Add( g4event, i, bc_id, px, py, pz, e ) ){
        xAOD::TruthParticle *par = new xAOD::TruthParticle( 
            m_p_pdg_id->at(i),
            seed_id,
            e*MeV, 
            pt*MeV, 
            eta, 
            m_p_phi->at(i), 
            px*MeV, 
            py*MeV, 
            pz*MeV, 
            m_p_prod_x->at(i)*mm,
            m_p_prod_y->at(i)*mm,
            m_p_prod_z->at(i)*mm
        );
        particles->push_back(par);
    }
  }

  return num_of_seeds;
}

bool EventReader::Add( G4Event* g4event , int i, int bc_id,
                       float px, float py, float pz, float e )
{
  G4LorentzVector xvtx( m_p_prod_x->at(i)*mm,
                        m_p_prod_y->at(i)*mm,
                        m_p_prod_z->at(i)*mm,
                        (m_p_prod_t->at(i) * mm / (c_light*ns)) + (bc_id*m_bc_duration) );
  G4PrimaryVertex* g4vtx= new G4PrimaryVertex( xvtx.x(), xvtx.y(), xvtx.z(), xvtx.t() );

  if (! CheckVertexInsideWorld(xvtx.vect())){
    std::cerr << "Particle is not inside the world!" << std::endl;
    std::cerr << "Pythia : " 
        << " x: " << m_p_prod_x->at(i)
        << " y: " << m_p_prod_y->at(i)
        << " z: " << m_p_prod_z->at(i)
        << " t[mm]: " << m_p_prod_t->at(i)
        << " pgd_id: " << m_p_pdg_id->at(i)
        << std::endl;
    delete g4vtx;   // evita vazamento quando a partícula não entra no mundo
    return false;
  }

  G4int pdgcode= m_p_pdg_id->at(i);
  G4LorentzVector p( px*GeV, py*GeV, pz*GeV, e*GeV );  // usa os valores locais
  G4PrimaryParticle* g4prim = new G4PrimaryParticle(pdgcode, p.x(), p.y(), p.z());
  g4vtx->SetPrimary(g4prim);
  g4event->AddPrimaryVertex(g4vtx);
  return true;
}
