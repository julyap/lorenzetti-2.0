#include "CaloCell/CaloCellContainer.h"
#include "EventInfo/EventInfoContainer.h"
#include "TruthParticle/TruthParticleContainer.h"
#include "CaloCell/CaloCellConverter.h"
#include "CaloCell/CaloDetDescriptorConverter.h"
#include "EventInfo/EventInfoConverter.h"
#include "TruthParticle/TruthParticleConverter.h"
#include "EventInfo/SeedConverter.h"
#include "TTree.h"
#include "RootStreamESDMaker.h"
#include "GaugiKernel/EDM.h"
#include "G4Kernel/CaloPhiRange.h"

// --- Aditivo: headers de muon ---
#include "MuonCell/MuonChamberCellContainer.h"
#include "MuonCell/MuonChamberConverter.h"
#include "MuonCell/MuonChamberDescriptorConverter.h"
#include "MuonCell/MuonChamberDetDescriptorCollection.h"

using namespace SG;
using namespace Gaugi;

/**
 * @class RootStreamESDMaker
 * @brief Serializes reconstruction objects to a ROOT file (ESD format).
 *
 * Similar to `RootStreamAODMaker`, but focuses on Event Summary Data (ESD),
 * which typically includes calorimeter cells. It supports filtering cells
 * within a Region of Interest (RoI) around truth particles to reduce file size.
 *
 * Properties:
 * - Eta/PhiWindow: Size of the RoI to save cells around truth particles.
 * - OnlyRoI: If true (default), only saves the cells inside the RoI; if false, saves all cells.
 *
 * --- Aditivo: muon ---
 * - InputMuonCellsKey: Key for MuonChamberCellContainer (single).
 * - InputMuonCellKeys: list of keys for multiple MuonChamberCellContainer.
 * - OutputMuonCellsKey: branch name for the muon cells.
 * - EnableCaloOutput/EnableMuonOutput: control the output.
 */
RootStreamESDMaker::RootStreamESDMaker( std::string name ) :
  IMsgService(name),
  Algorithm()
{
  // --- Oficiais (PRESERVADAS) ---
  declareProperty( "InputEventKey"      , m_inputEventKey="EventInfo"       );
  declareProperty( "InputTruthKey"      , m_inputTruthKey="Particles"       );
  declareProperty( "InputCellsKey"      , m_inputCellsKey="Cells"           );
  declareProperty( "InputSeedsKey"      , m_inputSeedsKey="Seeds"           );
  declareProperty( "InputCellsTruthKey" , m_inputCellsTruthKey="CellsTruth" );
  declareProperty( "OutputEventKey"     , m_outputEventKey="EventInfo"      );
  declareProperty( "OutputTruthKey"     , m_outputTruthKey="Particles"      );
  declareProperty( "OutputCellsKey"     , m_outputCellsKey="Cells"          );
  declareProperty( "OutputSeedsKey"     , m_outputSeedsKey="Seeds"          );
  declareProperty( "OutputCellsTruthKey", m_outputCellsTruthKey="CellsTruth");
  declareProperty( "OutputLevel"        , m_outputLevel=1                   );
  declareProperty( "NtupleName"         , m_ntupleName="CollectionTree"     );
  declareProperty( "EtaWindow"          , m_etaWindow=0.6                   );
  declareProperty( "PhiWindow"          , m_phiWindow=0.6                   );
  declareProperty( "OnlyRoI"            , m_onlyRoI=true                    );

  // --- Aditivo: aliases
  declareProperty( "InputCaloCellsKey"        , m_inputCaloCellsKey=""      );
  declareProperty( "InputTruthCaloCellsKey"   , m_inputTruthCaloCellsKey="" );
  declareProperty( "OutputCaloCellsKey"       , m_outputCaloCellsKey=""     );
  // ===== CORRIGIDO: alias simetrico para o output de truth =====
  declareProperty( "OutputTruthCaloCellsKey"  , m_outputTruthCaloCellsKey="");

  // --- Aditivo: muon ---
  declareProperty( "InputMuonCellsKey"      , m_inputMuonCellsKey=""        );
  declareProperty( "InputTruthMuonCellsKey" , m_inputTruthMuonCellsKey=""   );
  declareProperty( "InputMuonCellKeys"      , m_inputMuonCellKeys={}        );
  declareProperty( "OutputMuonCellsKey"     , m_outputMuonCellsKey="MuonCells" );

  // --- Aditivo: flags (default preservam o comportamento oficial) ---
  declareProperty( "EnableCaloOutput"       , m_enableCaloOutput=true       );
  declareProperty( "EnableMuonOutput"       , m_enableMuonOutput=true       );
}

//!=====================================================================

RootStreamESDMaker::~RootStreamESDMaker()
{;}

//!=====================================================================

StatusCode RootStreamESDMaker::initialize()
{
  CHECK_INIT();
  setMsgLevel(m_outputLevel);
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDMaker::bookHistograms( SG::EventContext &ctx ) const
{
  auto store = ctx.getStoreGateSvc();

  // --- Containers oficiais ---
  std::vector<xAOD::CaloCell_t            > container_cells;
  std::vector<xAOD::CaloCell_t            > container_cells_truth;
  std::vector<xAOD::CaloDetDescriptor_t   > container_descriptor;
  std::vector<xAOD::EventInfo_t           > container_event;
  std::vector<xAOD::Seed_t                > container_seeds;
  std::vector<xAOD::TruthParticle_t       > container_truth;

  // --- Containers aditivos de muon ---
  std::vector<xAOD::MuonChamber_t         > container_muon_cells;
  std::vector<xAOD::MuonChamberDescriptor_t> container_muon_descriptor;

  store->cd();
  TTree *tree = new TTree(m_ntupleName.c_str(), "");

  // --- Branches oficiais ---
  tree->Branch( ("EventInfoContainer_"         + m_outputEventKey).c_str() , &container_event                );
  tree->Branch( ("SeedContainer_"              + m_outputSeedsKey).c_str() , &container_seeds                );
  tree->Branch( ("TruthParticleContainer_"     + m_outputTruthKey).c_str() , &container_truth                );

  // Calorimeter cells: usa o alias se passado, senao o nome oficial
  const std::string out_cells       = m_outputCaloCellsKey.empty()      ? m_outputCellsKey      : m_outputCaloCellsKey;
  // ===== CORRIGIDO: alias de truth agora e resolvido simetricamente =====
  const std::string out_cells_truth = m_outputTruthCaloCellsKey.empty() ? m_outputCellsTruthKey : m_outputTruthCaloCellsKey;
  tree->Branch( ("CaloCellContainer_"          + out_cells).c_str()      , &container_cells                );
  tree->Branch( ("CaloCellContainer_"          + out_cells_truth).c_str(), &container_cells_truth          );
  tree->Branch( ("CaloDetDescriptorContainer_" + out_cells).c_str()      , &container_descriptor           );

  // --- Aditivo: muon ---
  if (m_enableMuonOutput) {
    tree->Branch( ("MuonChamberCellContainer_" + m_outputMuonCellsKey).c_str(), &container_muon_cells);
    tree->Branch( ("MuonChamberDetDescriptorContainer_" + m_outputMuonCellsKey).c_str(), &container_muon_descriptor);
  }

  store->add( tree );

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDMaker::pre_execute( EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDMaker::execute( EventContext &/*ctx*/, const G4Step * /*step*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDMaker::execute( EventContext &/*ctx*/, int /*evt*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDMaker::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDMaker::post_execute( EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDMaker::fillHistograms( EventContext &ctx ) const
{
  return serialize(ctx);
}

//!=====================================================================

template <class T>
void RootStreamESDMaker::InitBranch(TTree* fChain, std::string branch_name, T* param) const
{
  std::string bname = branch_name;
  if (fChain->GetAlias(bname.c_str()))
     bname = std::string(fChain->GetAlias(bname.c_str()));

  if (!fChain->FindBranch(bname.c_str()) ) {
    MSG_WARNING( "unknown branch " << bname );
    return;
  }
  fChain->SetBranchStatus(bname.c_str(), 1.);
  fChain->SetBranchAddress(bname.c_str(), param);
}

//!=====================================================================

StatusCode RootStreamESDMaker::serialize( EventContext &ctx ) const
{
  auto store = ctx.getStoreGateSvc();
  store->cd();
  TTree *tree = store->tree(m_ntupleName);

  // --- Containers oficiais ---
  std::vector<xAOD::CaloDetDescriptor_t > *container_descriptor   = nullptr;
  std::vector<xAOD::CaloCell_t          > *container_cells        = nullptr;
  std::vector<xAOD::CaloCell_t          > *container_cells_truth  = nullptr;
  std::vector<xAOD::EventInfo_t         > *container_event        = nullptr;
  std::vector<xAOD::Seed_t              > *container_seeds        = nullptr;
  std::vector<xAOD::TruthParticle_t     > *container_truth        = nullptr;

  // --- Containers aditivos de muon ---
  std::vector<xAOD::MuonChamber_t         > *container_muon_cells = nullptr;
  std::vector<xAOD::MuonChamberDescriptor_t> *container_muon_descriptor = nullptr;

  MSG_DEBUG( "Link all branches..." );

  // Resolve aliases: se o alias foi passado, tem precedencia
  const std::string in_cells        = m_inputCaloCellsKey.empty()      ? m_inputCellsKey      : m_inputCaloCellsKey;
  const std::string in_cells_truth  = m_inputTruthCaloCellsKey.empty() ? m_inputCellsTruthKey : m_inputTruthCaloCellsKey;
  const std::string out_cells       = m_outputCaloCellsKey.empty()     ? m_outputCellsKey     : m_outputCaloCellsKey;
  // ===== CORRIGIDO: resolver o alias de output de truth uma unica vez =====
  const std::string out_cells_truth = m_outputTruthCaloCellsKey.empty()? m_outputCellsTruthKey: m_outputTruthCaloCellsKey;

  // Branches oficiais
  InitBranch( tree, ("EventInfoContainer_"         + m_outputEventKey).c_str()      , &container_event        );
  InitBranch( tree, ("SeedContainer_"              + m_outputSeedsKey).c_str()      , &container_seeds        );
  InitBranch( tree, ("TruthParticleContainer_"     + m_outputTruthKey).c_str()      , &container_truth        );
  InitBranch( tree, ("CaloCellContainer_"          + out_cells).c_str()             , &container_cells        );
  // ===== CORRIGIDO: usar out_cells_truth (com alias resolvido) =====
  InitBranch( tree, ("CaloCellContainer_"          + out_cells_truth).c_str()       , &container_cells_truth  );
  InitBranch( tree, ("CaloDetDescriptorContainer_" + out_cells).c_str()             , &container_descriptor   );

  // Branches aditivas de muon
  if (m_enableMuonOutput) {
    InitBranch( tree, ("MuonChamberCellContainer_" + m_outputMuonCellsKey).c_str(), &container_muon_cells);
    InitBranch( tree, ("MuonChamberDetDescriptorContainer_" + m_outputMuonCellsKey).c_str(), &container_muon_descriptor);
  }

  { // serialize EventInfo -- IGUAL AO OFICIAL
    MSG_DEBUG("Serialize EventInfo...");
    SG::ReadHandle<xAOD::EventInfoContainer> event(m_inputEventKey, ctx);

    if( !event.isValid() ){
      MSG_FATAL( "It's not possible to read the xAOD::EventInfoContainer from this Context" );
    }

    xAOD::EventInfo_t event_t;
    xAOD::EventInfoConverter cnv;
    cnv.convert( (**event.ptr()).front(), event_t);
    container_event->push_back(event_t);
  }

  { // serialize Seed -- IGUAL AO OFICIAL
    MSG_DEBUG("Serialize Seed..");
    SG::ReadHandle<xAOD::SeedContainer> container( m_inputSeedsKey, ctx );

    if( !container.isValid() )
    {
      MSG_FATAL("It's not possible to read the xAOD::SeedContainer from this Context using this key " << m_inputSeedsKey );
    }

    for (const auto seed : **container.ptr() ){
      xAOD::Seed_t seed_t;
      xAOD::SeedConverter cnv;
      cnv.convert( seed, seed_t );
      container_seeds->push_back(seed_t);
    }
  }

  { // serialize CaloCells -- IGUAL AO OFICIAL (com alias)
    MSG_DEBUG("Serialize CaloCells...");

    SG::ReadHandle<xAOD::CaloCellContainer> container(in_cells, ctx);
    if( !container.isValid() )
    {
        MSG_FATAL("It's not possible to read the xAOD::CaloCellContainer from this Contaxt using this key " << in_cells );
    }

    SG::ReadHandle<xAOD::CaloCellContainer> cells_truth(in_cells_truth, ctx);
    if( !cells_truth.isValid() )
    {
        MSG_FATAL("It's not possible to read the xAOD::CaloCellContainer from this Contaxt using this key " << in_cells_truth );
    }

    SG::ReadHandle<xAOD::TruthParticleContainer> particles( m_inputTruthKey, ctx );

    if( !particles.isValid() )
    {
      MSG_FATAL("It's not possible to read the xAOD::TruthParticleContainer from this Context using this key " << m_inputTruthKey );
    }

    std::map<unsigned long int,const xAOD::CaloCell*> cell_truth_map;

    for (const auto cell : **cells_truth.ptr() ){
      cell_truth_map[cell->descriptor()->hash()] = cell;
    }

    std::map<unsigned long int,const xAOD::CaloCell*> cell_map;

    // Saves one cell (once, even when RoIs overlap) with its truth cell and its descriptor
    auto save = [&]( const xAOD::CaloCell *cell ){
        const xAOD::CaloDetDescriptor *descriptor = cell->descriptor();
        if ( cell_map.count(descriptor->hash())){
          return;
        }
        cell_map.insert( std::make_pair( descriptor->hash(), cell ) );

        if ( !cell_truth_map.count(descriptor->hash())){
          MSG_WARNING("Cell truth not found for cell " << descriptor->hash() );
          return;
        }

        const xAOD::CaloCell* cell_truth = cell_truth_map[descriptor->hash()];
        xAOD::CaloCell_t cell_truth_t;
        xAOD::CaloCellConverter cell_truth_cnv;
        cell_truth_cnv.convert(cell_truth, cell_truth_t);

        xAOD::CaloCell_t cell_t;
        xAOD::CaloCellConverter cell_cnv;
        cell_cnv.convert(cell, cell_t);

        xAOD::CaloDetDescriptor_t descriptor_t;
        xAOD::CaloDetDescriptorConverter descriptor_cnv;
        descriptor_cnv.convert( descriptor, descriptor_t);

        container_cells->push_back(cell_t);
        container_cells_truth->push_back(cell_truth_t);
        container_descriptor->push_back(descriptor_t);
    };

    if( !m_onlyRoI )
    {
        for (const auto cell : **container.ptr() ) save( cell );
    }
    else
    {
      for (const auto par : **particles.ptr() )
      {
        for (const auto cell : **container.ptr() ){
            const xAOD::CaloDetDescriptor *descriptor = cell->descriptor();
            float deltaEta = std::abs( par->eta() - descriptor->eta());
            float deltaPhi = std::abs( CaloPhiRange::diff(par->phi(), descriptor->phi()) );
            if ( deltaEta < m_etaWindow/2 && deltaPhi < m_phiWindow/2 ) save( cell );
        }// loop over all cells
      }// loop over all seeds
    }
  }

  // ============================================
  // Aditivo: Serialize Muon Cells
  // ============================================
  if (m_enableMuonOutput && container_muon_cells && container_muon_descriptor) {
    MSG_DEBUG("Serialize MuonCells...");

    std::set<unsigned long> processed_descriptors;

    if (!m_inputMuonCellKeys.empty()) {
      // Modo multi-key
      for (const auto& key : m_inputMuonCellKeys) {
        SG::ReadHandle<xAOD::MuonChamberCellContainer> muon_cells_container(key, ctx);
        if (!muon_cells_container.isValid()) {
          MSG_WARNING("  Cannot read MuonChamberCellContainer with key " << key);
          continue;
        }
        for (const auto cell : **muon_cells_container.ptr()) {
          if (!cell) continue;
          const xAOD::MuonChamberDescriptor* descriptor = cell->descriptor();
          if (descriptor) {
            if (processed_descriptors.find(descriptor->hash()) != processed_descriptors.end()) continue;
            processed_descriptors.insert(descriptor->hash());
          }
          xAOD::MuonChamber_t cell_t;
          xAOD::MuonChamberConverter cell_cnv;
          cell_cnv.convert(cell, cell_t);

          if (descriptor) {
            cell_t.sampling    = descriptor->sampling();
            cell_t.detector    = descriptor->chamberType();
            cell_t.chamberType = descriptor->chamberType();
            cell_t.region      = descriptor->region();
            cell_t.layer       = descriptor->layer();
          }
          container_muon_cells->push_back(cell_t);
          if (descriptor) {
            xAOD::MuonChamberDescriptor_t descriptor_t;
            xAOD::MuonChamberDescriptorConverter descriptor_cnv;
            descriptor_cnv.convert(descriptor, descriptor_t);
            container_muon_descriptor->push_back(descriptor_t);
          }
        }
      }
    }
    else if (!m_inputMuonCellsKey.empty()) {
      // Modo single-key
      SG::ReadHandle<xAOD::MuonChamberCellContainer> muon_cells_container(m_inputMuonCellsKey, ctx);
      if (muon_cells_container.isValid()) {
        for (const auto cell : **muon_cells_container.ptr()) {
          if (!cell) continue;
          const xAOD::MuonChamberDescriptor* descriptor = cell->descriptor();
          xAOD::MuonChamber_t cell_t;
          xAOD::MuonChamberConverter cell_cnv;
          cell_cnv.convert(cell, cell_t);
          if (descriptor) {
            cell_t.sampling    = descriptor->sampling();
            cell_t.detector    = descriptor->chamberType();
            cell_t.chamberType = descriptor->chamberType();
            cell_t.region      = descriptor->region();
            cell_t.layer       = descriptor->layer();
          }
          container_muon_cells->push_back(cell_t);
          if (descriptor) {
            xAOD::MuonChamberDescriptor_t descriptor_t;
            xAOD::MuonChamberDescriptorConverter descriptor_cnv;
            descriptor_cnv.convert(descriptor, descriptor_t);
            container_muon_descriptor->push_back(descriptor_t);
          }
        }
      } else {
        MSG_WARNING("Cannot read MuonChamberCellContainer with key " << m_inputMuonCellsKey);
      }
    }
  }

  { // serialize Truth Particle -- IGUAL AO OFICIAL
    MSG_DEBUG("Serialize TruthParticle...");
    SG::ReadHandle<xAOD::TruthParticleContainer> container( m_inputTruthKey, ctx );

    if( !container.isValid() )
    {
      MSG_FATAL("It's not possible to read the xAOD::TruthParticleContainer from this Context using this key " << m_inputTruthKey );
    }

    for (const auto par : **container.ptr() ){
      xAOD::TruthParticle_t par_t;
      xAOD::TruthParticleConverter cnv;
      cnv.convert( par, par_t );
      container_truth->push_back(par_t);
    }
  }

  tree->Fill();

  delete container_descriptor ;
  delete container_cells      ;
  delete container_cells_truth;
  delete container_seeds      ;
  delete container_event      ;
  delete container_truth      ;
  // Muon
  if (container_muon_cells) delete container_muon_cells;
  if (container_muon_descriptor) delete container_muon_descriptor;

  return StatusCode::SUCCESS;
}
