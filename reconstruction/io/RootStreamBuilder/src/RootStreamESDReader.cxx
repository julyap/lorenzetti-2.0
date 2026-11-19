
#include "CaloCell/CaloCellContainer.h"
#include "EventInfo/EventInfoContainer.h"
#include "TruthParticle/TruthParticleContainer.h"

#include "CaloCell/CaloCellConverter.h"
#include "CaloCell/CaloDetDescriptorConverter.h"
#include "CaloCell/CaloDetDescriptorContainer.h"

#include "EventInfo/EventInfoConverter.h"
#include "EventInfo/SeedConverter.h"

#include "TruthParticle/TruthParticleConverter.h"

#include "RootStreamESDReader.h"
#include "GaugiKernel/EDM.h"

// Muon
#include "MuonCell/MuonChamberCellContainer.h"
#include "MuonCell/MuonChamberConverter.h"
#include "MuonCell/MuonChamberDescriptorConverter.h"
#include "MuonCell/MuonChamberDescriptorContainer.h"

#include <map>
#include <set>
#include <memory>
#include <string>

#include <TFile.h>
#include <TTree.h>

using namespace SG;
using namespace Gaugi;


// ============================================================
// Constructor
// ============================================================

RootStreamESDReader::RootStreamESDReader(std::string name) :
  IMsgService(name),
  Algorithm()
{

  // Propriedades oficiais

  declareProperty("InputFile", m_inputFile = "");

  declareProperty("OutputEventKey", m_eventKey = "EventInfo");

  // NOVO:
  // Nome da branch de entrada no arquivo ROOT.
  // Não deve ser confundido com OutputEventKey.

  declareProperty(
    "InputEventBranchKey",
    m_inputEventBranchKey = "EventInfo"
  );

  declareProperty("OutputTruthKey", m_truthKey = "Particles");

  declareProperty("OutputCellsKey", m_cellsKey = "Cells");

  declareProperty(
    "OutputCellsTruthKey",
    m_cellsTruthKey = "CellsTruth"
  );

  declareProperty("OutputSeedsKey", m_seedsKey = "Seeds");

  declareProperty("OutputLevel", m_outputLevel = 1);

  declareProperty("NtupleName", m_ntupleName = "CollectionTree");


  // Aliases do calorímetro

  declareProperty(
    "OutputCaloCellsKey",
    m_calocellsKey = ""
  );

  declareProperty(
    "OutputTruthCaloCellsKey",
    m_truthCaloCellsKey = ""
  );


  // Muon

  declareProperty(
    "OutputMuonCellsKey",
    m_muonCellsKey = "MuonCells"
  );

  declareProperty(
    "OutputMuonCellKeys",
    m_muonCellKeys = {}
  );


  // Flags

  declareProperty(
    "EnableCaloOutput",
    m_enableCaloOutput = true
  );

  declareProperty(
    "EnableMuonOutput",
    m_enableMuonOutput = true
  );

}


// ============================================================
// Destructor
// ============================================================

RootStreamESDReader::~RootStreamESDReader()
{}


// ============================================================
// Initialize
// ============================================================

StatusCode RootStreamESDReader::initialize()
{

  CHECK_INIT();

  setMsgLevel(m_outputLevel);

  return StatusCode::SUCCESS;

}


// ============================================================
// Finalize
// ============================================================

StatusCode RootStreamESDReader::finalize()
{

  return StatusCode::SUCCESS;

}


// ============================================================
// Book Histograms
// ============================================================

StatusCode RootStreamESDReader::bookHistograms(
  EventContext &ctx
) const
{

  auto store = ctx.getStoreGateSvc();

  TFile *file = new TFile(
    m_inputFile.c_str(),
    "read"
  );

  if (!file || file->IsZombie()) {

    MSG_FATAL(
      "Cannot open input file: "
      << m_inputFile
    );

    return StatusCode::FAILURE;

  }

  store->decorate("events", file);

  return StatusCode::SUCCESS;

}


// ============================================================
// Pre Execute
// ============================================================

StatusCode RootStreamESDReader::pre_execute(
  EventContext &/*ctx*/
) const
{

  return StatusCode::SUCCESS;

}


// ============================================================
// Execute G4Step
// ============================================================

StatusCode RootStreamESDReader::execute(
  EventContext &/*ctx*/,
  const G4Step * /*step*/
) const
{

  return StatusCode::SUCCESS;

}


// ============================================================
// Execute Event
// ============================================================

StatusCode RootStreamESDReader::execute(
  EventContext &ctx,
  int evt
) const
{

  return deserialize(evt, ctx);

}


// ============================================================
// Post Execute
// ============================================================

StatusCode RootStreamESDReader::post_execute(
  EventContext &/*ctx*/
) const
{

  return StatusCode::SUCCESS;

}


// ============================================================
// Fill Histograms
// ============================================================

StatusCode RootStreamESDReader::fillHistograms(
  EventContext &/*ctx*/
) const
{

  return StatusCode::SUCCESS;

}


// ============================================================
// Deserialize
// ============================================================

StatusCode RootStreamESDReader::deserialize(
  int evt,
  EventContext &ctx
) const
{

  // ----------------------------------------------------------
  // Containers persistentes
  // ----------------------------------------------------------

  std::vector<xAOD::CaloDetDescriptor_t>* collection_descriptor = nullptr;

  std::vector<xAOD::CaloCell_t>* collection_cells = nullptr;

  std::vector<xAOD::CaloCell_t>* collection_cells_truth = nullptr;

  std::vector<xAOD::EventInfo_t>* collection_event = nullptr;

  std::vector<xAOD::Seed_t>* collection_seeds = nullptr;

  std::vector<xAOD::TruthParticle_t>* collection_truth = nullptr;


  // ----------------------------------------------------------
  // Containers de múons
  // ----------------------------------------------------------

  std::vector<xAOD::MuonChamber_t>* collection_muon_cells = nullptr;

  std::vector<xAOD::MuonChamberDescriptor_t>* collection_muon_descriptor = nullptr;


  MSG_DEBUG("Link all branches...");


  // ----------------------------------------------------------
  // StoreGate e ROOT
  // ----------------------------------------------------------

  auto store = ctx.getStoreGateSvc();

  TFile *file = (TFile*)store->decorator("events");

  if (!file || file->IsZombie()) {

    MSG_FATAL("Invalid ROOT input file.");

    return StatusCode::FAILURE;

  }

  TTree *tree = (TTree*)file->Get(m_ntupleName.c_str());

  if (!tree) {

    MSG_FATAL(
      "Cannot find tree: "
      << m_ntupleName
    );

    return StatusCode::FAILURE;

  }


  // ----------------------------------------------------------
  // Resolver aliases do calorímetro
  // ----------------------------------------------------------

  const std::string cells_key =
    m_calocellsKey.empty()
    ? m_cellsKey
    : m_calocellsKey;

  const std::string cells_truth_key =
    m_truthCaloCellsKey.empty()
    ? m_cellsTruthKey
    : m_truthCaloCellsKey;


  // ----------------------------------------------------------
  // Branch EventInfo
  //
  // IMPORTANTE:
  // A branch ROOT usa m_inputEventBranchKey.
  // A saída StoreGate usa m_eventKey.
  // ----------------------------------------------------------

  InitBranch(
    tree,
    "EventInfoContainer_" + m_inputEventBranchKey,
    &collection_event
  );


  // ----------------------------------------------------------
  // Branches oficiais
  // ----------------------------------------------------------

  InitBranch(
    tree,
    "SeedContainer_" + m_seedsKey,
    &collection_seeds
  );

  InitBranch(
    tree,
    "TruthParticleContainer_" + m_truthKey,
    &collection_truth
  );


  // ----------------------------------------------------------
  // Branches do calorímetro
  // ----------------------------------------------------------

  if (m_enableCaloOutput) {

    InitBranch(
      tree,
      "CaloCellContainer_" + cells_key,
      &collection_cells
    );

    InitBranch(
      tree,
      "CaloCellContainer_" + cells_truth_key,
      &collection_cells_truth
    );

    InitBranch(
      tree,
      "CaloDetDescriptorContainer_" + cells_key,
      &collection_descriptor
    );

  }


  // ----------------------------------------------------------
  // Branches de múons
  // ----------------------------------------------------------

  if (m_enableMuonOutput) {

    InitBranch(
      tree,
      "MuonChamberCellContainer_" + m_muonCellsKey,
      &collection_muon_cells
    );

    InitBranch(
      tree,
      "MuonChamberDetDescriptorContainer_" + m_muonCellsKey,
      &collection_muon_descriptor
    );

  }


  // ----------------------------------------------------------
  // Ler evento
  // ----------------------------------------------------------

  tree->GetEntry(evt);


  // ----------------------------------------------------------
  // Validar containers obrigatórios
  // ----------------------------------------------------------

  if (!collection_event || collection_event->empty()) {

    MSG_FATAL(
      "EventInfo branch is missing or empty. "
      << "Expected branch: EventInfoContainer_"
      << m_inputEventBranchKey
    );

    return StatusCode::FAILURE;

  }


  if (!collection_truth) {

    MSG_FATAL(
      "TruthParticle branch is missing."
    );

    return StatusCode::FAILURE;

  }


  if (!collection_seeds) {

    MSG_FATAL(
      "Seed branch is missing."
    );

    return StatusCode::FAILURE;

  }


  if (
    m_enableCaloOutput &&
    (
      !collection_cells ||
      !collection_cells_truth ||
      !collection_descriptor
    )
  ) {

    MSG_FATAL(
      "One or more calorimeter branches are missing."
    );

    return StatusCode::FAILURE;

  }


  // ----------------------------------------------------------
  // TruthParticle
  // ----------------------------------------------------------

  MSG_DEBUG("Deserialize TruthParticle...");

  {

    SG::WriteHandle<xAOD::TruthParticleContainer> container(
      m_truthKey,
      ctx
    );

    container.record(
      std::unique_ptr<xAOD::TruthParticleContainer>(
        new xAOD::TruthParticleContainer()
      )
    );

    xAOD::TruthParticleConverter cnv;

    for (auto& par_t : *collection_truth) {

      xAOD::TruthParticle *par = nullptr;

      cnv.convert(par_t, par);

      MSG_DEBUG(
        "Particle in eta = "
        << par->eta()
        << ", phi = "
        << par->phi()
      );

      container->push_back(par);

    }

  }


  // ----------------------------------------------------------
  // EventInfo
  // ----------------------------------------------------------

  MSG_DEBUG("Deserialize EventInfo...");

  {

    SG::WriteHandle<xAOD::EventInfoContainer> container(
      m_eventKey,
      ctx
    );

    container.record(
      std::unique_ptr<xAOD::EventInfoContainer>(
        new xAOD::EventInfoContainer()
      )
    );

    xAOD::EventInfo *event = nullptr;

    xAOD::EventInfoConverter cnv;

    cnv.convert(
      collection_event->at(0),
      event
    );

    MSG_DEBUG(
      "EventNumber = "
      << event->eventNumber()
      << ", Avgmu = "
      << event->avgmu()
    );

    container->push_back(event);

  }


  // ----------------------------------------------------------
  // Seeds
  // ----------------------------------------------------------

  MSG_DEBUG("Deserialize Seed...");

  {

    SG::WriteHandle<xAOD::SeedContainer> container(
      m_seedsKey,
      ctx
    );

    container.record(
      std::unique_ptr<xAOD::SeedContainer>(
        new xAOD::SeedContainer()
      )
    );

    xAOD::SeedConverter cnv;

    for (auto& seed_t : *collection_seeds) {

      xAOD::Seed *seed = nullptr;

      cnv.convert(seed_t, seed);

      MSG_DEBUG(
        "Seed in eta = "
        << seed->eta()
        << ", phi = "
        << seed->phi()
      );

      container->push_back(seed);

    }

  }


  // ----------------------------------------------------------
  // Calorimeter descriptors e cells
  // ----------------------------------------------------------

  if (
    m_enableCaloOutput &&
    collection_cells &&
    collection_descriptor
  ) {

    MSG_DEBUG("Deserialize CaloDetDescriptor...");

    xAOD::descriptor_links_t descriptor_links;


    // --------------------------------------------------------
    // CaloDetDescriptor
    // --------------------------------------------------------

    {

      SG::WriteHandle<xAOD::CaloDetDescriptorContainer> container(
        cells_key + "_Aux",
        ctx
      );

      container.record(
        std::unique_ptr<xAOD::CaloDetDescriptorContainer>(
          new xAOD::CaloDetDescriptorContainer()
        )
      );

      std::map<
        unsigned long int,
        const xAOD::CaloDetDescriptor_t
      > descriptor_map;

      MSG_DEBUG(collection_descriptor->size());

      for (auto &descriptor_t : *collection_descriptor) {

        if (descriptor_map.count(descriptor_t.hash)) {
          continue;
        }

        descriptor_map.insert(
          std::make_pair(
            descriptor_t.hash,
            descriptor_t
          )
        );

        xAOD::CaloDetDescriptor *descriptor = nullptr;

        xAOD::CaloDetDescriptorConverter cnv;

        cnv.convert(descriptor_t, descriptor);

        descriptor_links[descriptor->hash()] = descriptor;

        container->push_back(descriptor);

      }

    }


    // --------------------------------------------------------
    // CaloCells
    // --------------------------------------------------------

    MSG_DEBUG("Deserialize CaloCells...");

    {

      SG::WriteHandle<xAOD::CaloCellContainer> container(
        cells_key,
        ctx
      );

      container.record(
        std::unique_ptr<xAOD::CaloCellContainer>(
          new xAOD::CaloCellContainer()
        )
      );

      std::map<
        unsigned long int,
        const xAOD::CaloDetDescriptor*
      > descriptor_map;

      for (auto &cell_t : *collection_cells) {

        if (descriptor_links.count(cell_t.descriptor_link)) {

          auto descriptor =
            descriptor_links[cell_t.descriptor_link];

          if (descriptor_map.count(descriptor->hash())) {

            MSG_FATAL(
              "There is a cell duplication. Please check the code. Abort"
            );

            continue;

          }

          descriptor_map.insert(
            std::make_pair(
              descriptor->hash(),
              descriptor
            )
          );

          xAOD::CaloCell *cell = nullptr;

          xAOD::CaloCellConverter cnv;

          cnv.convert(cell_t, cell);

          cell->setDescriptor(descriptor);

          container->push_back(cell);

        }

      }

    }


    // --------------------------------------------------------
    // CaloCells Truth
    // --------------------------------------------------------

    MSG_DEBUG("Deserialize CaloCells Truth...");

    {

      SG::WriteHandle<xAOD::CaloCellContainer> container(
        cells_truth_key,
        ctx
      );

      container.record(
        std::unique_ptr<xAOD::CaloCellContainer>(
          new xAOD::CaloCellContainer()
        )
      );

      std::map<
        unsigned long int,
        const xAOD::CaloDetDescriptor*
      > descriptor_map;

      for (auto &cell_t : *collection_cells_truth) {

        if (descriptor_links.count(cell_t.descriptor_link)) {

          auto descriptor =
            descriptor_links[cell_t.descriptor_link];

          if (descriptor_map.count(descriptor->hash())) {

            MSG_FATAL(
              "There is a cell duplication. Please check the code. Abort"
            );

            continue;

          }

          descriptor_map.insert(
            std::make_pair(
              descriptor->hash(),
              descriptor
            )
          );

          xAOD::CaloCell *cell = nullptr;

          xAOD::CaloCellConverter cnv;

          cnv.convert(cell_t, cell);

          cell->setDescriptor(descriptor);

          container->push_back(cell);

        }

      }

    }

  }


  // ----------------------------------------------------------
  // Muon chamber descriptors e cells
  // ----------------------------------------------------------

  if (m_enableMuonOutput) {

    if (collection_muon_cells && collection_muon_descriptor) {

      MSG_DEBUG("Deserialize MuonChamberCells...");


      // ------------------------------------------------------
      // Muon descriptors
      // ------------------------------------------------------

      SG::WriteHandle<xAOD::MuonChamberDescriptorContainer> container_desc(
        m_muonCellsKey + "_Aux",
        ctx
      );

      container_desc.record(
        std::unique_ptr<xAOD::MuonChamberDescriptorContainer>(
          new xAOD::MuonChamberDescriptorContainer()
        )
      );

      std::map<
        unsigned long,
        const xAOD::MuonChamberDescriptor*
      > descriptor_links;

      xAOD::MuonChamberDescriptorConverter desc_cnv;

      std::set<unsigned long> processed;

      for (auto &desc_t : *collection_muon_descriptor) {

        if (processed.count(desc_t.hash)) {
          continue;
        }

        processed.insert(desc_t.hash);

        xAOD::MuonChamberDescriptor *desc = nullptr;

        desc_cnv.convert(desc_t, desc);

        descriptor_links[desc->hash()] = desc;

        container_desc->push_back(desc);

      }


      // ------------------------------------------------------
      // Muon cells
      // ------------------------------------------------------

      SG::WriteHandle<xAOD::MuonChamberCellContainer> container(
        m_muonCellsKey,
        ctx
      );

      container.record(
        std::unique_ptr<xAOD::MuonChamberCellContainer>(
          new xAOD::MuonChamberCellContainer()
        )
      );

      xAOD::MuonChamberConverter cell_cnv;

      for (auto &cell_t : *collection_muon_cells) {

        xAOD::MuonChamberCell *cell = nullptr;

        cell_cnv.convert(cell_t, cell);

        if (descriptor_links.count(cell_t.descriptor_link)) {

          cell->setDescriptor(
            descriptor_links[cell_t.descriptor_link]
          );

        }

        container->push_back(cell);

      }

      MSG_DEBUG(
        "Deserialized "
        << container->size()
        << " muon cells"
      );

    }
    else {

      MSG_WARNING(
        "Muon branches are not available in this ESD file. "
        "Muon deserialization will be skipped."
      );

    }

  }


  // ----------------------------------------------------------
  // Liberar containers persistentes
  // ----------------------------------------------------------

  delete collection_descriptor;

  delete collection_seeds;

  delete collection_cells;

  delete collection_cells_truth;

  delete collection_event;

  delete collection_truth;

  delete collection_muon_cells;

  delete collection_muon_descriptor;


  return StatusCode::SUCCESS;

}


// ============================================================
// InitBranch
// ============================================================

template <class T>
void RootStreamESDReader::InitBranch(
  TTree* fChain,
  std::string branch_name,
  T* param
) const
{

  std::string bname = branch_name;

  if (fChain->GetAlias(bname.c_str())) {

    bname = std::string(
      fChain->GetAlias(bname.c_str())
    );

  }

  if (!fChain->FindBranch(bname.c_str())) {

    MSG_WARNING(
      "unknown branch "
      << bname
    );

    *param = nullptr;

    return;

  }

  fChain->SetBranchStatus(
    bname.c_str(),
    1.
  );

  fChain->SetBranchAddress(
    bname.c_str(),
    param
  );

}

