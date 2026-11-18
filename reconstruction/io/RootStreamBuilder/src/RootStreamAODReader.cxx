#include "CaloCell/CaloCellContainer.h"
#include "EventInfo/EventInfoContainer.h"
#include "TruthParticle/TruthParticleContainer.h"
#include "CaloCluster/CaloClusterContainer.h"
#include "CaloRings/CaloRingsContainer.h"
#include "Egamma/ElectronContainer.h"

#include "CaloCell/CaloCellConverter.h"
#include "CaloCell/CaloDetDescriptorConverter.h"
#include "CaloCell/CaloDetDescriptorCollection.h"
#include "EventInfo/EventInfoConverter.h"
#include "TruthParticle/TruthParticleConverter.h"
#include "EventInfo/SeedConverter.h"
#include "CaloCluster/CaloClusterConverter.h"
#include "CaloRings/CaloRingsConverter.h"
#include "Egamma/ElectronConverter.h"

// --- Aditivo: headers de muon ---
#include "Muon/MuonContainer.h"
#include "MuonCell/MuonChamberCellContainer.h"
#include "MuonCluster/MuonClusterContainer.h"
#include "Muon/MuonConverter.h"
#include "MuonCell/MuonChamberConverter.h"
#include "MuonCell/MuonChamberDescriptorConverter.h"
#include "MuonCell/MuonChamberDescriptorContainer.h"
#include "MuonCluster/MuonClusterConverter.h"

#include "RootStreamAODReader.h"
#include "GaugiKernel/EDM.h"

using namespace SG;
using namespace Gaugi;

/**
 * @class RootStreamAODReader
 * @brief Reads AOD data from a ROOT file and reconstructs xAOD objects.
 *
 * This algorithm is the inverse of `RootStreamAODMaker`. It opens a ROOT file,
 * reads the persistent structs from the TTree, and converts them back into
 * transient `xAOD` objects (EventInfo, CaloClusters, Electrons, etc.), which
 * are then recorded into StoreGate for downstream algorithms.
 *
 * Properties:
 * - InputFile: Path to the ROOT file.
 * - Output*Key: Keys to record objects in StoreGate.
 *
 * --- Aditivo: muon ---
 * - OutputMuonKey, OutputMuonClusterKey: Keys for muon objects.
 * - OutputMuonCellsKey, OutputMuonDescriptorKey: Keys for muon chamber cells.
 * - EnableCaloOutput, EnableMuonOutput, EnableMuonCellsOutput: controls.
 */
RootStreamAODReader::RootStreamAODReader(std::string name) : IMsgService(name),
                                                             Algorithm()
{
  // --- Oficiais (PRESERVADAS) ---
  declareProperty("OutputEventKey", m_eventKey = "EventInfo");
  declareProperty("OutputSeedsKey", m_seedsKey = "Seeds");
  declareProperty("OutputTruthKey", m_truthKey = "Particles");
  declareProperty("OutputClusterKey", m_clusterKey = "Clusters");
  declareProperty("OutputRingerKey", m_ringerKey = "Rings");
  declareProperty("OutputRingerL0Key", m_ringerL0Key = "RingsL0");
  declareProperty("OutputElectronKey", m_electronKey = "Electrons");
  declareProperty("OutputTruthClusterKey", m_truthClusterKey = "TruthClusters");
  declareProperty("OutputTruthElectronKey", m_truthElectronKey = "TruthElectrons");
  declareProperty("OutputLevel", m_outputLevel = 1);
  declareProperty("NtupleName", m_ntupleName = "CollectionTree");
  declareProperty("InputFile", m_inputFile = "");

  declareProperty("OutputRingerKeys", m_ringerKeys = {"Rings"});
  declareProperty("OutputTruthRingerKeys", m_truthRingerKeys = {"TruthRings"});

  // --- Aditivo: muon ---
  declareProperty("OutputMuonKey", m_muonKey = "Muons");
  declareProperty("OutputMuonClusterKey", m_muonClusterKey = "MuonClusters");
  // ===== CORRIGIDO: chaves de MuonChamberCellContainer e descriptor =====
  declareProperty("OutputMuonCellsKey", m_muonCellsKey = "MuonCells");
  declareProperty("OutputMuonDescriptorKey", m_muonDescriptorKey = "MuonCells");

  // --- Aditivo: flags (default preservam o comportamento oficial) ---
  declareProperty("EnableCaloOutput", m_enableCaloOutput = true);
  declareProperty("EnableMuonOutput", m_enableMuonOutput = true);
  // ===== CORRIGIDO: default true preserva o comportamento atual,
  //     mas o usuario pode desligar se o AOD nao contiver MuonChamberCells =====
  declareProperty("EnableMuonCellsOutput", m_enableMuonCellsOutput = true);
}

//!=====================================================================

RootStreamAODReader::~RootStreamAODReader()
{
}

//!=====================================================================

StatusCode RootStreamAODReader::initialize()
{
  CHECK_INIT();
  setMsgLevel(m_outputLevel);
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODReader::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODReader::bookHistograms(EventContext &ctx) const
{
  auto store = ctx.getStoreGateSvc();
  TFile *file = new TFile(m_inputFile.c_str(), "read");
  if (!file || file->IsZombie()) {
    MSG_FATAL("Cannot open input file: " << m_inputFile);
    return StatusCode::FAILURE;
  }
  store->decorate("events", file);
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODReader::pre_execute(EventContext & /*ctx*/) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODReader::execute(EventContext & /*ctx*/, const G4Step * /*step*/) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODReader::execute(EventContext &ctx, int evt) const
{
  return deserialize(evt, ctx);
}

//!=====================================================================

StatusCode RootStreamAODReader::post_execute(EventContext & /*ctx*/) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODReader::fillHistograms(EventContext &ctx) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODReader::deserialize(int evt, EventContext &ctx) const
{
  // --- Containers oficiais ---
  std::vector<xAOD::CaloDetDescriptor_t> *collection_descriptor = nullptr;
  std::vector<xAOD::CaloCell_t> *collection_cells = nullptr;
  std::vector<xAOD::EventInfo_t> *collection_event = nullptr;
  std::vector<xAOD::Seed_t> *collection_seeds = nullptr;
  std::vector<xAOD::TruthParticle_t> *collection_truth = nullptr;
  std::vector<xAOD::CaloCluster_t> *collection_clus = nullptr;
  std::vector<xAOD::Electron_t> *collection_el = nullptr;
  std::vector<xAOD::CaloCluster_t> *collection_truth_clus = nullptr;
  std::vector<xAOD::CaloRings_t> *collection_rings_l0 = nullptr;
  std::vector<xAOD::Electron_t> *collection_truth_el = nullptr;

  // --- Containers aditivos de muon ---
  std::vector<xAOD::Muon_t> *collection_muon = nullptr;
  std::vector<xAOD::MuonChamber_t> *collection_muon_cells = nullptr;
  std::vector<xAOD::MuonChamberDescriptor_t> *collection_muon_descriptor = nullptr;
  std::vector<xAOD::MuonCluster_t> *collection_muon_clusters = nullptr;

  MSG_DEBUG("Link all branches...");

  auto store = ctx.getStoreGateSvc();
  TFile *file = (TFile *)store->decorator("events");
  TTree *tree = (TTree *)file->Get(m_ntupleName.c_str());

  if (!tree) {
    MSG_FATAL("Cannot find tree: " << m_ntupleName);
    return StatusCode::FAILURE;
  }

  // --- Branches oficiais ---
  InitBranch(tree, ("EventInfoContainer_" + m_eventKey).c_str(), &collection_event);
  InitBranch(tree, ("SeedContainer_" + m_seedsKey).c_str(), &collection_seeds);
  InitBranch(tree, ("TruthParticleContainer_" + m_truthKey).c_str(), &collection_truth);
  InitBranch(tree, ("CaloClusterContainer_" + m_clusterKey).c_str(), &collection_clus);
  InitBranch(tree, ("ElectronContainer_" + m_electronKey).c_str(), &collection_el);
  InitBranch(tree, ("CaloClusterContainer_" + m_truthClusterKey).c_str(), &collection_truth_clus);
  InitBranch(tree, ("CaloRingsContainer_" + m_ringerL0Key).c_str(), &collection_rings_l0);
  InitBranch(tree, ("ElectronContainer_" + m_truthElectronKey).c_str(), &collection_truth_el);

  // --- Ringer keys (OFICIAL, PRESERVADO) ---
  std::vector<std::vector<xAOD::CaloRings_t> *> collections_rings(m_ringerKeys.size(), nullptr);
  std::vector<std::vector<xAOD::CaloRings_t> *> collections_truth_rings(m_truthRingerKeys.size(), nullptr);

  for (size_t i = 0; i < m_ringerKeys.size(); ++i)
    InitBranch(tree, ("CaloRingsContainer_" + m_ringerKeys[i]).c_str(), &collections_rings[i]);
  for (size_t i = 0; i < m_truthRingerKeys.size(); ++i)
    InitBranch(tree, ("CaloRingsContainer_" + m_truthRingerKeys[i]).c_str(), &collections_truth_rings[i]);

  // --- Branches aditivas de muon ---
  if (m_enableMuonOutput) {
    InitBranch(tree, ("MuonContainer_" + m_muonKey).c_str(), &collection_muon);
    InitBranch(tree, ("MuonClusterContainer_" + m_muonClusterKey).c_str(), &collection_muon_clusters);
    // ===== CORRIGIDO: ler MuonChamberCells/Descriptor apenas se habilitado =====
    if (m_enableMuonCellsOutput) {
      InitBranch(tree, ("MuonChamberCellContainer_" + m_muonCellsKey).c_str(), &collection_muon_cells);
      InitBranch(tree, ("MuonChamberDetDescriptorContainer_" + m_muonDescriptorKey).c_str(), &collection_muon_descriptor);
    }
  }

  tree->GetEntry(evt);

  // --- Deserialize TruthParticle ---
  if (collection_truth) {
    MSG_DEBUG("Deserialize TruthParticle...");
    SG::WriteHandle<xAOD::TruthParticleContainer> container(m_truthKey, ctx);
    container.record(std::unique_ptr<xAOD::TruthParticleContainer>(new xAOD::TruthParticleContainer()));

    xAOD::TruthParticleConverter cnv;
    for (auto &par_t : *collection_truth) {
      xAOD::TruthParticle *par = nullptr;
      cnv.convert(par_t, par);
      container->push_back(par);
    }
  }

  // --- Deserialize EventInfo ---
  if (collection_event && !collection_event->empty()) {
    MSG_DEBUG("Deserialize EventInfo...");
    SG::WriteHandle<xAOD::EventInfoContainer> container(m_eventKey, ctx);
    container.record(std::unique_ptr<xAOD::EventInfoContainer>(new xAOD::EventInfoContainer()));
    xAOD::EventInfo *event = nullptr;
    xAOD::EventInfoConverter cnv;
    cnv.convert(collection_event->at(0), event);
    container->push_back(event);
  }

  // --- Deserialize Seeds ---
  xAOD::seed_links_t seed_links;
  if (collection_seeds) {
    MSG_DEBUG("Deserialize Seed...");
    SG::WriteHandle<xAOD::SeedContainer> container(m_seedsKey, ctx);
    container.record(std::unique_ptr<xAOD::SeedContainer>(new xAOD::SeedContainer()));

    xAOD::SeedConverter cnv;
    for (auto &seed_t : *collection_seeds) {
      xAOD::Seed *seed = nullptr;
      cnv.convert(seed_t, seed);
      container->push_back(seed);
      seed_links[seed->id()] = seed;
    }
  }

  // --- Deserialize Calorimeter (Clusters, Rings, Electrons) ---
  if (m_enableCaloOutput && collection_clus && collection_el) {
    MSG_DEBUG("Deserialize Clusters, Rings and Electrons...");

    SG::WriteHandle<xAOD::CaloClusterContainer> container_clus(m_clusterKey, ctx);
    container_clus.record(std::unique_ptr<xAOD::CaloClusterContainer>(new xAOD::CaloClusterContainer()));

    SG::WriteHandle<xAOD::ElectronContainer> container_el(m_electronKey, ctx);
    container_el.record(std::unique_ptr<xAOD::ElectronContainer>(new xAOD::ElectronContainer()));

    SG::WriteHandle<xAOD::CaloClusterContainer> container_truth_clus(m_truthClusterKey, ctx);
    container_truth_clus.record(std::unique_ptr<xAOD::CaloClusterContainer>(new xAOD::CaloClusterContainer()));

    SG::WriteHandle<xAOD::ElectronContainer> container_truth_el(m_truthElectronKey, ctx);
    container_truth_el.record(std::unique_ptr<xAOD::ElectronContainer>(new xAOD::ElectronContainer()));

    xAOD::CaloClusterConverter clus_cnv;
    xAOD::ElectronConverter el_cnv;
    xAOD::cluster_links_t clus_links;

    // Clusters digitais
    for (auto &clus_t : *collection_clus) {
      xAOD::CaloCluster *clus = nullptr;
      clus_cnv.convert(clus_t, clus);
      if (seed_links.find(clus_t.seed_link) != seed_links.end()) {
        clus->setSeed(seed_links[clus_t.seed_link]);
      }
      container_clus->push_back(clus);
      if (clus->seed()) {
        clus_links[clus->seed()->id()] = clus;
      }
    }

    // Eletrons digitais
    for (auto &el_t : *collection_el) {
      xAOD::Electron *el = nullptr;
      el_cnv.convert(el_t, el);
      if (clus_links.find(el_t.cluster_link) != clus_links.end()) {
        el->setCaloCluster(clus_links[el_t.cluster_link]);
      }
      container_el->push_back(el);
    }

    // Clusters de verdade
    xAOD::cluster_links_t truth_clus_links;
    for (auto &truth_clus_t : *collection_truth_clus) {
      xAOD::CaloCluster *truth_clus = nullptr;
      clus_cnv.convert(truth_clus_t, truth_clus);
      if (seed_links.find(truth_clus_t.seed_link) != seed_links.end()) {
        truth_clus->setSeed(seed_links[truth_clus_t.seed_link]);
        truth_clus_links[truth_clus->seed()->id()] = truth_clus;
      }
      container_truth_clus->push_back(truth_clus);
    }

    // Eletrons de verdade
    for (auto &el_t : *collection_truth_el) {
      xAOD::Electron *el = nullptr;
      el_cnv.convert(el_t, el);
      if (truth_clus_links.find(el_t.cluster_link) != truth_clus_links.end()) {
        el->setCaloCluster(truth_clus_links[el_t.cluster_link]);
      }
      container_truth_el->push_back(el);
    }

    // Rings (loop por chave) — OFICIAL PRESERVADO
    xAOD::CaloRingsConverter rings_cnv;
    for (size_t i = 0; i < m_ringerKeys.size(); ++i) {
      SG::WriteHandle<xAOD::CaloRingsContainer> container_rings(m_ringerKeys[i], ctx);
      container_rings.record(std::make_unique<xAOD::CaloRingsContainer>());
      if (collections_rings[i]) {
        for (auto &rings_t : *collections_rings[i]) {
          xAOD::CaloRings *rings = nullptr;
          rings_cnv.convert(rings_t, rings);
          if (clus_links.find(rings_t.cluster_link) != clus_links.end())
            rings->setCaloCluster(clus_links[rings_t.cluster_link]);
          container_rings->push_back(rings);
        }
      }
    }

    for (size_t i = 0; i < m_truthRingerKeys.size(); ++i) {
      SG::WriteHandle<xAOD::CaloRingsContainer> container_truth_rings(m_truthRingerKeys[i], ctx);
      container_truth_rings.record(std::make_unique<xAOD::CaloRingsContainer>());
      if (collections_truth_rings[i]) {
        for (auto &rings_t : *collections_truth_rings[i]) {
          xAOD::CaloRings *rings = nullptr;
          rings_cnv.convert(rings_t, rings);
          if (truth_clus_links.find(rings_t.cluster_link) != truth_clus_links.end())
            rings->setCaloCluster(truth_clus_links[rings_t.cluster_link]);
          container_truth_rings->push_back(rings);
        }
      }
    }
  }

  // --- Deserialize Muon System ---
  if (m_enableMuonOutput) {

    // ===== CORRIGIDO: MuonChamberCells + Descriptor (antes eram lidos e descartados) =====
    if (m_enableMuonCellsOutput && collection_muon_cells) {
      MSG_DEBUG("Deserialize MuonChamberCells...");

      // Deserializa os descriptors primeiro
      std::map<unsigned long, const xAOD::MuonChamberDescriptor*> descriptor_links;
      if (collection_muon_descriptor) {
        SG::WriteHandle<xAOD::MuonChamberDescriptorContainer> container_desc(m_muonDescriptorKey + "_Aux", ctx);
        container_desc.record(std::unique_ptr<xAOD::MuonChamberDescriptorContainer>(new xAOD::MuonChamberDescriptorContainer()));

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

      // Deserializa as cells e linka com os descriptors
      SG::WriteHandle<xAOD::MuonChamberCellContainer> container_cells(m_muonCellsKey, ctx);
      container_cells.record(std::unique_ptr<xAOD::MuonChamberCellContainer>(new xAOD::MuonChamberCellContainer()));

      xAOD::MuonChamberConverter cell_cnv;
      for (auto &cell_t : *collection_muon_cells) {
        xAOD::MuonChamber *cell = nullptr;
        cell_cnv.convert(cell_t, cell);
        if (descriptor_links.count(cell_t.descriptor_link)) {
          cell->setDescriptor(descriptor_links[cell_t.descriptor_link]);
        }
        container_cells->push_back(cell);
      }
    }

    // Muon Clusters
    if (collection_muon_clusters) {
      MSG_DEBUG("Deserialize MuonClusters...");
      SG::WriteHandle<xAOD::MuonClusterContainer> container(m_muonClusterKey, ctx);
      container.record(std::unique_ptr<xAOD::MuonClusterContainer>(new xAOD::MuonClusterContainer()));
      xAOD::MuonClusterConverter cnv;
      for (auto &cluster_t : *collection_muon_clusters) {
        xAOD::MuonCluster *cluster = nullptr;
        cnv.convert(cluster_t, cluster);
        container->push_back(cluster);
      }
    }

    // Muons
    if (collection_muon) {
      MSG_DEBUG("Deserialize Muons...");
      SG::WriteHandle<xAOD::MuonContainer> container(m_muonKey, ctx);
      container.record(std::unique_ptr<xAOD::MuonContainer>(new xAOD::MuonContainer()));
      xAOD::MuonConverter cnv;
      for (auto &muon_t : *collection_muon) {
        xAOD::Muon *muon = nullptr;
        cnv.convert(muon_t, muon);
        container->push_back(muon);
      }
    }
  }

  // Cleanup
  delete collection_descriptor;
  delete collection_seeds;
  delete collection_cells;
  delete collection_event;
  delete collection_truth;
  delete collection_clus;
  delete collection_el;
  delete collection_truth_clus;
  delete collection_rings_l0;
  delete collection_truth_el;
  // Muon
  delete collection_muon;
  delete collection_muon_cells;
  delete collection_muon_descriptor;
  delete collection_muon_clusters;

  for (auto *c : collections_rings) delete c;
  for (auto *c : collections_truth_rings) delete c;

  return StatusCode::SUCCESS;
}

//!=====================================================================

template <class T>
void RootStreamAODReader::InitBranch(TTree *fChain, std::string branch_name, T *param) const
{
  std::string bname = branch_name;
  if (fChain->GetAlias(bname.c_str()))
    bname = std::string(fChain->GetAlias(bname.c_str()));

  if (!fChain->FindBranch(bname.c_str())) {
    MSG_WARNING("unknown branch " << bname);
    *param = nullptr;
    return;
  }
  fChain->SetBranchStatus(bname.c_str(), 1.);
  fChain->SetBranchAddress(bname.c_str(), param);
}
