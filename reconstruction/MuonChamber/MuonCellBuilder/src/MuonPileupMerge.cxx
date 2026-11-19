// ============================================================================
// Arquivo: MuonPileupMerge.cxx
// Descrição: Implementação do algoritmo MuonPileupMerge. Abre um arquivo
//            ROOT de pileup (InputFile), lê a ntuple (NtupleName) com as
//            coleções de hits (InputHitsKey) e de EventInfo (InputEventKey),
//            sorteia um evento e copia os hits e o EventInfo para as
//            coleções de saída (OutputHitsKey e OutputEventKey). O objetivo
//            é sobrepor eventos de pileup à simulação principal.
// ============================================================================

#include "MuonPileupMerge.h"
#include "MuonHit/MuonHitConverter.h"
#include "EventInfo/EventInfoConverter.h"

using namespace Gaugi;


MuonPileupMerge::MuonPileupMerge(std::string name) :
  Algorithm(),
  m_rng(0)
{
  // --- Propriedades configuráveis ---------------------------------
  declareProperty("InputFile"      , m_inputFile=""                       );
  declareProperty("InputHitsKey"   , m_inputHitsKey="MuonHits"            );
  declareProperty("OutputHitsKey"  , m_outputHitsKey="MuonHits_Merged"    );
  declareProperty("InputEventKey"  , m_inputEventKey="EventInfo"          );
  declareProperty("OutputEventKey" , m_outputEventKey="EventInfo_Merged"  );
  declareProperty("OutputLevel"    , m_outputLevel=1                      );
  declareProperty("NtupleName"     , m_ntupleName="CollectionTree"        );
  declareProperty("Seed"           , m_seed=0                             );
}

//!=====================================================================

MuonPileupMerge::~MuonPileupMerge()
{}

//!=====================================================================

StatusCode MuonPileupMerge::initialize()
{
  CHECK_INIT();
  setMsgLevel(m_outputLevel);
  m_rng.SetSeed(m_seed);
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonPileupMerge::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonPileupMerge::pre_execute()
{
  MSG_DEBUG("Pre-execute hook");
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonPileupMerge::execute()
{
  MSG_DEBUG("Execute main logic");

  // Abrir arquivo de pileup
  auto file = new TFile(m_inputFile.c_str(), "READ");
  if (!file || file->IsZombie()){
    MSG_FATAL("Falha ao abrir arquivo de pileup: " << m_inputFile);
    return StatusCode::FAILURE;
  }

  // Obter a TTree
  auto tree = (TTree*)file->Get(m_ntupleName.c_str());
  if (!tree){
    MSG_FATAL("Não achei a TTree " << m_ntupleName);
    file->Close();
    delete file;
    return StatusCode::FAILURE;
  }

  // Branches - usando tipos corretos baseados na interface real
  std::vector<xAOD::MuonHit> *collection_hits = nullptr;
  tree->SetBranchAddress(("MuonHitContainer_"+m_inputHitsKey).c_str(), &collection_hits);

  std::vector<xAOD::EventInfo> *collection_events = nullptr;
  tree->SetBranchAddress(("EventInfoContainer_"+m_inputEventKey).c_str(), &collection_events);

  // Escolhe evento aleatório
  int eventNumber = m_rng.Integer(tree->GetEntries());
  if (tree->GetEntry(eventNumber) <= 0){
    MSG_FATAL("Erro ao ler evento " << eventNumber);
    file->Close();
    delete file;
    return StatusCode::FAILURE;
  }

  // Container de saída
  auto hits_out = new xAOD::MuonHitContainer();
  auto events_out = new xAOD::EventInfoContainer();

  // Loop de hits - usando construtor com 12 argumentos (INCLUINDO REGION)
  for (const auto& hit_t : *collection_hits){
    auto hit = new xAOD::MuonHit(
      hit_t.x(),           // x
      hit_t.y(),           // y
      hit_t.z(),           // z
      hit_t.dx(),          // dx
      hit_t.dy(),          // dy
      hit_t.hash(),        // hash
      hit_t.sampling(),    // sampling
      hit_t.detector(),    // detector
      hit_t.region(),      // region  <-- NOVO PARÂMETRO ADICIONADO
      hit_t.bc_duration(), // bc_duration
      hit_t.bcid_start(),  // bcid_start
      hit_t.bcid_end(),     // bcid_end
      hit_t.energyDeposit() //  // <-- COPIA A ENERGIA!

    );

    // Copiar outros atributos
   // hit->setEnergyDeposit(hit_t.energyDeposit());
    hit->setTime(hit_t.time());
    if (!hit_t.pulse().empty()) {
      hit->setPulse(hit_t.pulse());
    }

    hits_out->push_back(hit);
  }

  // EventInfo - usando métodos disponíveis
  if (!collection_events->empty()) {
    auto evt = new xAOD::EventInfo();
    const auto& event_t = collection_events->at(0);

    // Usar métodos disponíveis da interface EventInfo
    evt->setEventNumber(event_t.eventNumber());
    // Outros métodos conforme disponíveis na interface EventInfo

    events_out->push_back(evt);
  }

  MSG_INFO("Merge concluído: " << hits_out->size() << " hits e " << events_out->size() << " eventos.");

  // Limpeza
  file->Close();
  delete file;

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonPileupMerge::post_execute()
{
  MSG_DEBUG("Post-execute hook");
  return StatusCode::SUCCESS;
}
