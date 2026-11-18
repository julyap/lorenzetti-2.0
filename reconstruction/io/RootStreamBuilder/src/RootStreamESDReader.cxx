#include "CaloCell/CaloCellContainer.h"
#include "EventInfo/EventInfoContainer.h"
#include "TruthParticle/TruthParticleContainer.h"
#include "CaloCell/CaloCellConverter.h"
#include "CaloCell/CaloDetDescriptorConverter.h"
#include "CaloCell/CaloDetDescriptorContainer.h"
#include "EventInfo/EventInfoConverter.h"
#include "TruthParticle/TruthParticleConverter.h"
#include "EventInfo/SeedConverter.h"
#include "RootStreamESDReader.h"
#include "GaugiKernel/EDM.h"

// --- Aditivo: headers de muon ---
#include "MuonCell/MuonChamberCellContainer.h"
#include "MuonCell/MuonChamberConverter.h"
#include "MuonCell/MuonChamberDescriptorConverter.h"
#include "MuonCell/MuonChamberDetDescriptorContainer.h"
#include "MuonCell/MuonChamberDescriptorContainer.h"

using namespace SG;
using namespace Gaugi;

/**
 * @class RootStreamESDReader
 * @brief Reads ESD data from a ROOT file.
 *
 * Reconstructs `xAOD::CaloCell` objects (and their descriptors) from the
 * persistent data. It handles the restoration of cell-descriptor links.
 *
 * --- Aditivo: muon ---
 * Also reads muon chamber cells (MuonChamberCellContainer) and their
 * descriptors (MuonChamberDescriptorContainer) when EnableMuonOutput is true.
 */
RootStreamESDReader::RootStreamESDReader( std::string name ) :
  IMsgService(name),
  Algorithm()
{
  // --- Oficiais (PRESERVADAS) ---
  declareProperty( "InputFile"          , m_inputFile=""                    );
  declareProperty( "OutputEventKey"     , m_eventKey="EventInfo"            );
  declareProperty( "OutputTruthKey"     , m_truthKey="Particles"            );
  declareProperty( "OutputCellsKey"     , m_cellsKey="Cells"                );
  declareProperty( "OutputCellsTruthKey", m_cellsTruthKey="CellsTruth"      );
  declareProperty( "OutputSeedsKey"     , m_seedsKey="Seeds"                );
  declareProperty( "OutputLevel"        , m_outputLevel=1                   );
  declareProperty( "NtupleName"         , m_ntupleName="CollectionTree"     );

  // --- Aditivo: aliases da sua versao ---
  declareProperty( "OutputCaloCellsKey"      , m_calocellsKey=""            );
  // ===== CORRIGIDO: alias simetrico para o output de truth =====
  declareProperty( "OutputTruthCaloCellsKey" , m_truthCaloCellsKey=""       );

  // --- Aditivo: muon ---
  declareProperty( "OutputMuonCellsKey" , m_muonCellsKey="MuonCells"        );
  declareProperty( "OutputMuonCellKeys" , m_muonCellKeys={}                 );

  // --- Aditivo: flags (default preservam o comportamento oficial) ---
  declareProperty( "EnableCaloOutput"   , m_enableCaloOutput=true           );
  declareProperty( "EnableMuonOutput"   , m_enableMuonOutput=true           );
}

//!=====================================================================

RootStreamESDReader::~RootStreamESDReader()
{}

//!=====================================================================

StatusCode RootStreamESDReader::initialize()
{
  CHECK_INIT();
  setMsgLevel(m_outputLevel);
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDReader::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDReader::bookHistograms( EventContext &ctx ) const
{
  auto store = ctx.getStoreGateSvc();
  TFile *file = new TFile(m_inputFile.c_str(), "read");
  if (!file || file->IsZombie()) {
    MSG_FATAL("Cannot open input file: " << m_inputFile);
    return StatusCode::FAILURE;
  }
  store->decorate( "events", file );
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDReader::pre_execute( EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDReader::execute( EventContext &/*ctx*/, const G4Step * /*step*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDReader::execute( EventContext &ctx, int evt ) const
{
  return deserialize( evt, ctx );
}

//!=====================================================================

StatusCode RootStreamESDReader::post_execute( EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDReader::fillHistograms( EventContext &ctx ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamESDReader::deserialize( int evt, EventContext &ctx ) const
{
  // --- Containers oficiais ---
  std::vector<xAOD::CaloDetDescriptor_t > *collection_descriptor    = nullptr;
  std::vector<xAOD::CaloCell_t          > *collection_cells         = nullptr;
  std::vector<xAOD::CaloCell_t          > *collection_cells_truth   = nullptr;
  std::vector<xAOD::EventInfo_t         > *collection_event         = nullptr;
  std::vector<xAOD::Seed_t              > *collection_seeds         = nullptr;
  std::vector<xAOD::TruthParticle_t     > *collection_truth         = nullptr;

  // --- Containers aditivos de muon ---
  std::vector<xAOD::MuonChamber_t         > *collection_muon_cells = nullptr;
  std::vector<xAOD::MuonChamberDescriptor_t> *collection_muon_descriptor = nullptr;

  MSG_DEBUG( "Link all branches..." );

  auto store = ctx.getStoreGateSvc();
  TFile *file = (TFile*)store->decorator("events");
  TTree *tree = (TTree*)file->Get(m_ntupleName.c_str());

  if (!tree) {
    MSG_FATAL("Cannot find tree: " << m_ntupleName);
    return StatusCode::FAILURE;
  }

  // ===== CORRIGIDO: resolver aliases simetricamente ao ESDMaker =====
  const std::string cells_key       = m_calocellsKey.empty()      ? m_cellsKey       : m_calocellsKey;
  const std::string cells_truth_key = m_truthCaloCellsKey.empty() ? m_cellsTruthKey  : m_truthCaloCellsKey;

  // --- Branches oficiais ---
  InitBranch( tree, ("EventInfoContainer_"         + m_eventKey).c_str()      , &collection_event       );
  InitBranch( tree, ("SeedContainer_"              + m_seedsKey).c_str()      , &collection_seeds       );
  InitBranch( tree, ("TruthParticleContainer_"     + m_truthKey).c_str()      , &collection_truth       );
  InitBranch( tree, ("CaloCellContainer_"          + cells_key).c_str()       , &collection_cells       );
  // ===== CORRIGIDO: usar cells_truth_key (com alias resolvido) =====
  InitBranch( tree, ("CaloCellContainer_"          + cells_truth_key).c_str() , &collection_cells_truth );
  InitBranch( tree, ("CaloDetDescriptorContainer_" + cells_key).c_str()       , &collection_descriptor  );

  // --- Branches aditivas de muon ---
  if (m_enableMuonOutput) {
    InitBranch( tree, ("MuonChamberCellContainer_" + m_muonCellsKey).c_str(), &collection_muon_cells);
    InitBranch( tree, ("MuonChamberDetDescriptorContainer_" + m_muonCellsKey).c_str(), &collection_muon_descriptor);
  }

  tree->GetEntry( evt );

  MSG_DEBUG("Deserialize TruthParticle...");

  { // deserialize TruthParticle -- IGUAL AO OFICIAL
    SG::WriteHandle<xAOD::TruthParticleContainer> container(m_truthKey, ctx);
    container.record( std::unique_ptr<xAOD::TruthParticleContainer>(new xAOD::TruthParticleContainer()));

    xAOD::TruthParticleConverter cnv;
    for( auto& par_t : *collection_truth)
    {
      xAOD::TruthParticle  *par=nullptr;
      cnv.convert(par_t, par);
      MSG_DEBUG( "Particle in eta = " << par->eta() << ", phi = " << par->phi());
      container->push_back(par);
    }
  }

  MSG_DEBUG("Deserialize EventInfo...");

  { // deserialize EventInfo -- IGUAL AO OFICIAL
    SG::WriteHandle<xAOD::EventInfoContainer> container(m_eventKey, ctx);
    container.record( std::unique_ptr<xAOD::EventInfoContainer>(new xAOD::EventInfoContainer()));
    xAOD::EventInfo  *event=nullptr;
    xAOD::EventInfoConverter cnv;
    cnv.convert(  collection_event->at(0), event);
    MSG_DEBUG( "EventNumber = " << event->eventNumber() << ", Avgmu = " << event->avgmu());
    container->push_back(event);
  }

  MSG_DEBUG("Deserialize Seed...");
  { // deserialize Seed -- IGUAL AO OFICIAL
    SG::WriteHandle<xAOD::SeedContainer> container(m_seedsKey, ctx);
    container.record( std::unique_ptr<xAOD::SeedContainer>(new xAOD::SeedContainer()));

    xAOD::SeedConverter cnv;
    for( auto& seed_t : *collection_seeds)
    {
      xAOD::Seed  *seed=nullptr;
      cnv.convert(seed_t, seed);
      MSG_DEBUG( "Seed in eta = " << seed->eta() << ", phi = " << seed->phi());
      container->push_back(seed);
    }
  }

  // ============================================
  // Deserialize CaloDetDescriptor / CaloCells (OFICIAL PRESERVADO)
  // ============================================
  if (m_enableCaloOutput && collection_cells && collection_descriptor) {
    MSG_DEBUG("Deserialize CaloDetDescriptor... ");
    xAOD::descriptor_links_t descriptor_links;

    {
      SG::WriteHandle<xAOD::CaloDetDescriptorContainer> container( cells_key+"_Aux", ctx );
      container.record( std::unique_ptr<xAOD::CaloDetDescriptorContainer>(new xAOD::CaloDetDescriptorContainer()) );
      std::map<unsigned long int,const xAOD::CaloDetDescriptor_t> descriptor_map;
      MSG_DEBUG(collection_descriptor->size());
      for (auto &descriptor_t : *collection_descriptor )
      {
        if ( descriptor_map.count(descriptor_t.hash)){
          continue;
        }
        descriptor_map.insert( std::make_pair( descriptor_t.hash, descriptor_t ) );
        xAOD::CaloDetDescriptor *descriptor = nullptr;
        xAOD::CaloDetDescriptorConverter cnv;
        cnv.convert(descriptor_t, descriptor); // alloc memory
        descriptor_links[descriptor->hash()] = descriptor;
        container->push_back(descriptor);
      }
    }

    MSG_DEBUG("Deserialize CaloCells... ");
    {
      SG::WriteHandle<xAOD::CaloCellContainer> container(cells_key, ctx);
      container.record( std::unique_ptr<xAOD::CaloCellContainer>(new xAOD::CaloCellContainer()));
      std::map<unsigned long int,const xAOD::CaloDetDescriptor*> descriptor_map;
      for( auto &cell_t : *collection_cells )
      {
        if ( descriptor_links.count(cell_t.descriptor_link)  )
        {
          auto descriptor = descriptor_links[cell_t.descriptor_link];
          if ( descriptor_map.count(descriptor->hash())){
            MSG_FATAL("There is a cell duplication. Please check the code. Abort");
            continue;
          }
          descriptor_map.insert( std::make_pair( descriptor->hash(), descriptor ) );
          xAOD::CaloCell *cell = nullptr;
          xAOD::CaloCellConverter cnv;
          cnv.convert(cell_t, cell); // alloc memory
          cell->setDescriptor( descriptor );
          container->push_back(cell);
        }
      }
    }

    MSG_DEBUG("Deserialize CaloCells Truth... ");
    {
      // ===== CORRIGIDO: usar cells_truth_key (com alias resolvido) =====
      SG::WriteHandle<xAOD::CaloCellContainer> container(cells_truth_key, ctx);
      container.record( std::unique_ptr<xAOD::CaloCellContainer>(new xAOD::CaloCellContainer()));
      std::map<unsigned long int,const xAOD::CaloDetDescriptor*> descriptor_map;
      for( auto &cell_t : *collection_cells_truth )
      {
        if ( descriptor_links.count(cell_t.descriptor_link)  )
        {
          auto descriptor = descriptor_links[cell_t.descriptor_link];
          if ( descriptor_map.count(descriptor->hash())){
            MSG_FATAL("There is a cell duplication. Please check the code. Abort");
            continue;
          }
          descriptor_map.insert( std::make_pair( descriptor->hash(), descriptor ) );
          xAOD::CaloCell *cell = nullptr;
          xAOD::CaloCellConverter cnv;
          cnv.convert(cell_t, cell); // alloc memory
          cell->setDescriptor( descriptor );
          container->push_back(cell);
        }
      }
    }
  }

  // ============================================
  // Aditivo: Deserialize Muon Cells
  // ============================================
  if (m_enableMuonOutput && collection_muon_cells) {
    MSG_DEBUG("Deserialize MuonChamberCells...");

    // Deserialize descriptors primeiro
    SG::WriteHandle<xAOD::MuonChamberDescriptorContainer> container_desc(m_muonCellsKey + "_Aux", ctx);
    container_desc.record( std::unique_ptr<xAOD::MuonChamberDescriptorContainer>(new xAOD::MuonChamberDescriptorContainer()) );

    std::map<unsigned long, const xAOD::MuonChamberDescriptor*> descriptor_links;
    if (collection_muon_descriptor) {
      xAOD::MuonChamberDescriptorConverter desc_cnv;
      std::set<unsigned long> processed;
      for (auto &desc_t : *collection_muon_descriptor) {
        if (processed.count(desc_t.hash)) continue;
        processed.insert(desc_t.hash);
        xAOD::MuonChamberDescriptor *desc = nullptr;
        desc_cnv.convert(desc_t, desc);
        descriptor_links[desc->hash()] = desc;
        container_desc->push_back(desc);
      }
    }

    // Deserialize cells
    SG::WriteHandle<xAOD::MuonChamberCellContainer> container(m_muonCellsKey, ctx);
    container.record( std::unique_ptr<xAOD::MuonChamberCellContainer>(new xAOD::MuonChamberCellContainer()) );

    xAOD::MuonChamberConverter cell_cnv;
    for (auto &cell_t : *collection_muon_cells) {
      xAOD::MuonChamber *cell = nullptr;
      cell_cnv.convert(cell_t, cell);
      if (descriptor_links.count(cell_t.descriptor_link)) {
        cell->setDescriptor(descriptor_links[cell_t.descriptor_link]);
      }
      container->push_back(cell);
    }
    MSG_DEBUG("Deserialized " << container->size() << " muon cells");
  }

  delete collection_descriptor  ;
  delete collection_seeds       ;
  delete collection_cells       ;
  delete collection_cells_truth ;
  delete collection_event       ;
  delete collection_truth       ;
  // Muon
  if (collection_muon_cells) delete collection_muon_cells;
  if (collection_muon_descriptor) delete collection_muon_descriptor;

  return StatusCode::SUCCESS;
}

//!=====================================================================

template <class T>
void RootStreamESDReader::InitBranch(TTree* fChain, std::string branch_name, T* param) const
{
  std::string bname = branch_name;
  if (fChain->GetAlias(bname.c_str()))
     bname = std::string(fChain->GetAlias(bname.c_str()));

  if (!fChain->FindBranch(bname.c_str()) ) {
    MSG_WARNING( "unknown branch " << bname );
    *param = nullptr;
    return;
  }
  fChain->SetBranchStatus(bname.c_str(), 1.);
  fChain->SetBranchAddress(bname.c_str(), param);
}
