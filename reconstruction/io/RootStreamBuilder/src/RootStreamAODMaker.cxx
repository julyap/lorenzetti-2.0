#include "CaloCell/CaloCellContainer.h"
#include "CaloCluster/CaloClusterContainer.h"
#include "CaloRings/CaloRingsContainer.h"
#include "EventInfo/EventInfoContainer.h"
#include "TruthParticle/TruthParticleContainer.h"
#include "Egamma/ElectronContainer.h"

#include "CaloCell/CaloCellConverter.h"
#include "CaloCell/CaloDetDescriptorConverter.h"
#include "CaloCluster/CaloClusterConverter.h"
#include "CaloRings/CaloRingsConverter.h"
#include "EventInfo/EventInfoConverter.h"
#include "EventInfo/SeedConverter.h"
#include "TruthParticle/TruthParticleConverter.h"
#include "Egamma/ElectronConverter.h"

// --- Aditivo: headers de muon (todos existem no oficial, confirmado por `find`) ---
#include "Muon/MuonContainer.h"
#include "MuonCell/MuonChamberCellContainer.h"
#include "MuonCluster/MuonClusterContainer.h"
#include "Muon/MuonConverter.h"
#include "MuonCell/MuonChamberConverter.h"
#include "MuonCell/MuonChamberDescriptorConverter.h"
#include "MuonCluster/MuonClusterConverter.h"

#include "TTree.h"
#include "RootStreamAODMaker.h"
#include "GaugiKernel/EDM.h"

using namespace SG;
using namespace Gaugi;

/**
 * @class RootStreamAODMaker
 * @brief Serializes reconstruction objects to a ROOT file (AOD format).
 *
 * This algorithm gathers various EDM objects (EventInfo, TruthParticles,
 * CaloClusters, CaloRings, Electrons, etc.) from StoreGate and writes them
 * into a TTree. It uses "Converter" helper classes to translate transient
 * xAOD objects into persistent struct-based representations.
 *
 * Properties:
 * - Input*Key: Keys for objects in StoreGate.
 * - Output*Key: Branch names in the output TTree.
 * - NtupleName: Name of the TTree.
 * - DumpCells: If true, also dumps detailed cell information (heavy).
 *
 * --- Aditivo: muon ---
 * - InputMuonKey, InputMuonCellsKey, InputMuonClusterKey: Keys for muon objects.
 * - OutputMuonKey, OutputMuonCellsKey, OutputMuonClusterKey: Branch names.
 * - EnableMuonOutput: If true, writes the muon branches (default true).
 * - DumpMuonCells, DumpMuonClusters: If true, dumps muon cells/clusters.
 */
RootStreamAODMaker::RootStreamAODMaker(std::string name) : IMsgService(name),
                                                           Algorithm()
{
  // --- Oficiais (PRESERVADOS) ---
  declareProperty("InputEventKey", m_inputEventKey = "EventInfo");
  declareProperty("InputSeedsKey", m_inputSeedsKey = "Seeds");
  declareProperty("InputTruthKey", m_inputTruthKey = "Particles");

  declareProperty("InputCellsKey", m_inputCellsKey = "Cells");
  declareProperty("InputClusterKey", m_inputClusterKey = "Clusters");
  declareProperty("InputRingerKey", m_inputRingerKey = "Rings");
  declareProperty("InputElectronKey", m_inputElectronKey = "Electrons");
  declareProperty("InputRingerL0Key", m_inputRingerL0Key = "RingsL0");

  declareProperty("InputTruthCellsKey", m_inputTruthCellsKey = "TruthCells");
  declareProperty("InputTruthClusterKey", m_inputTruthClusterKey = "TruthClusters");
  declareProperty("InputTruthRingerKey", m_inputTruthRingerKey = "TruthRings");
  declareProperty("InputTruthElectronKey", m_inputTruthElectronKey = "TruthElectrons");

  declareProperty("OutputEventKey", m_outputEventKey = "EventInfo");
  declareProperty("OutputSeedsKey", m_outputSeedsKey = "Seeds");
  declareProperty("OutputTruthKey", m_outputTruthKey = "Particles");

  declareProperty("OutputCellsKey", m_outputCellsKey = "Cells");
  declareProperty("OutputClusterKey", m_outputClusterKey = "Clusters");
  declareProperty("OutputRingerKey", m_outputRingerKey = "Rings");
  declareProperty("OutputElectronKey", m_outputElectronKey = "Electrons");
  declareProperty("OutputRingerL0Key", m_outputRingerL0Key = "RingsL0");

  declareProperty("OutputTruthCellsKey", m_outputTruthCellsKey = "TruthCells");
  declareProperty("OutputTruthClusterKey", m_outputTruthClusterKey = "TruthClusters");
  declareProperty("OutputTruthRingerKey", m_outputTruthRingerKey = "TruthRings");
  declareProperty("OutputTruthElectronKey", m_outputTruthElectronKey = "TruthElectrons");

  declareProperty("OutputLevel", m_outputLevel = 1);
  declareProperty("NtupleName", m_ntupleName = "physics");
  declareProperty("DumpCells", m_dumpCells = false);

  declareProperty("InputRingerKeys", m_inputRingerKeys = {"Rings"});
  declareProperty("InputTruthRingerKeys", m_inputTruthRingerKeys = {"TruthRings"});
  declareProperty("OutputRingerKeys", m_outputRingerKeys = {"Rings"});
  declareProperty("OutputTruthRingerKeys", m_outputTruthRingerKeys = {"TruthRings"});

  // --- Aditivo: muon ---
  declareProperty("InputMuonKey", m_inputMuonKey = "Muons");
  declareProperty("InputMuonCellsKey", m_inputMuonCellsKey = "MuonCells");
  declareProperty("InputMuonClusterKey", m_inputMuonClusterKey = "MuonClusters");

  declareProperty("OutputMuonKey", m_outputMuonKey = "Muons");
  declareProperty("OutputMuonCellsKey", m_outputMuonCellsKey = "MuonCells");
  declareProperty("OutputMuonClusterKey", m_outputMuonClusterKey = "MuonClusters");

  declareProperty("EnableCaloOutput", m_enableCaloOutput = true);
  declareProperty("EnableMuonOutput", m_enableMuonOutput = true);
  declareProperty("DumpMuonCells", m_dumpMuonCells = false);
  declareProperty("DumpMuonClusters", m_dumpMuonClusters = false);
}

//!=====================================================================

RootStreamAODMaker::~RootStreamAODMaker()
{
  ;
}

//!=====================================================================

StatusCode RootStreamAODMaker::initialize()
{
  CHECK_INIT();
  setMsgLevel(m_outputLevel);
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODMaker::bookHistograms(SG::EventContext &ctx) const
{
  auto store = ctx.getStoreGateSvc();

  // --- Containers oficiais (PRESERVADOS) ---
  std::vector<xAOD::EventInfo_t> container_event;
  std::vector<xAOD::Seed_t> container_seeds;
  std::vector<xAOD::TruthParticle_t> container_truth;

  std::vector<xAOD::CaloCell_t> container_cells;
  std::vector<xAOD::CaloDetDescriptor_t> container_descriptor;
  std::vector<xAOD::CaloCluster_t> container_clus;
  std::vector<xAOD::CaloRings_t> container_rings;
  std::vector<xAOD::CaloRings_t> container_ringsL0;
  std::vector<xAOD::Electron_t> container_electron;

  std::vector<xAOD::CaloCell_t> container_cells_truth;
  std::vector<xAOD::CaloCluster_t> container_clus_truth;
  std::vector<xAOD::CaloRings_t> container_rings_truth;
  std::vector<xAOD::Electron_t> container_electron_truth;

  // --- Containers aditivos de muon ---
  std::vector<xAOD::Muon_t> container_muon;
  std::vector<xAOD::MuonChamber_t> container_muon_cells;
  std::vector<xAOD::MuonChamberDescriptor_t> container_muon_descriptor;
  std::vector<xAOD::MuonCluster_t> container_muon_clusters;

  store->cd();
  TTree *tree = new TTree(m_ntupleName.c_str(), "");

  tree->Branch(("EventInfoContainer_" + m_outputEventKey).c_str(), &container_event);
  tree->Branch(("SeedContainer_" + m_outputSeedsKey).c_str(), &container_seeds);
  tree->Branch(("TruthParticleContainer_" + m_outputTruthKey).c_str(), &container_truth);

  // tree->Branch(("CaloRingsContainer_" + m_outputRingerKey).c_str(), &container_rings);
  tree->Branch(("CaloRingsContainer_" + m_outputRingerL0Key).c_str(), &container_ringsL0);
  tree->Branch(("CaloClusterContainer_" + m_outputClusterKey).c_str(), &container_clus);
  tree->Branch(("ElectronContainer_" + m_outputElectronKey).c_str(), &container_electron);

  tree->Branch(("CaloClusterContainer_" + m_outputTruthClusterKey).c_str(), &container_clus_truth);
  // tree->Branch(("CaloRingsContainer_" + m_outputTruthRingerKey).c_str(), &container_rings_truth);
  tree->Branch(("ElectronContainer_" + m_outputTruthElectronKey).c_str(), &container_electron_truth);

  // --- Ringer keys (OFICIAL, PRESERVADO) ---
  for (const auto &key : m_outputRingerKeys)
  {
    std::vector<xAOD::CaloRings_t> tmp;
    tree->Branch(("CaloRingsContainer_" + key).c_str(), &tmp);
  }
  for (const auto &key : m_outputTruthRingerKeys)
  {
    std::vector<xAOD::CaloRings_t> tmp;
    tree->Branch(("CaloRingsContainer_" + key).c_str(), &tmp);
  }

  if (m_dumpCells)
  {
    tree->Branch(("CaloCellContainer_" + m_outputCellsKey).c_str(), &container_cells);
    tree->Branch(("CaloDetDescriptorContainer_" + m_outputCellsKey).c_str(), &container_descriptor);
    tree->Branch(("CaloCellContainer_" + m_outputTruthCellsKey).c_str(), &container_cells_truth);
  }

  // --- Aditivo: branches de muon ---
  if (m_enableMuonOutput) {
    tree->Branch(("MuonContainer_" + m_outputMuonKey).c_str(), &container_muon);
    if (m_dumpMuonCells) {
      tree->Branch(("MuonChamberCellContainer_" + m_outputMuonCellsKey).c_str(), &container_muon_cells);
      tree->Branch(("MuonChamberDetDescriptorContainer_" + m_outputMuonCellsKey).c_str(), &container_muon_descriptor);
    }
    if (m_dumpMuonClusters) {
      tree->Branch(("MuonClusterContainer_" + m_outputMuonClusterKey).c_str(), &container_muon_clusters);
    }
  }

  store->add(tree);

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODMaker::pre_execute(EventContext & /*ctx*/) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODMaker::execute(EventContext & /*ctx*/, const G4Step * /*step*/) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODMaker::execute(EventContext & /*ctx*/, int /*evt*/) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODMaker::post_execute(EventContext & /*ctx*/) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode RootStreamAODMaker::fillHistograms(EventContext &ctx) const
{
  return serialize(ctx);
}

//!=====================================================================

template <class T>
void RootStreamAODMaker::InitBranch(TTree *fChain, std::string branch_name, T *param) const
{
  std::string bname = branch_name;
  if (fChain->GetAlias(bname.c_str()))
    bname = std::string(fChain->GetAlias(bname.c_str()));

  if (!fChain->FindBranch(bname.c_str()))
  {
    MSG_WARNING("unknown branch " << bname);
    return;
  }
  fChain->SetBranchStatus(bname.c_str(), 1.);
  fChain->SetBranchAddress(bname.c_str(), param);
}

//!=====================================================================

StatusCode RootStreamAODMaker::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================
// Metodos oficiais de serializacao (PRESERVADOS)
//!=====================================================================

void RootStreamAODMaker::serializeCaloCluster(EventContext &ctx,
                                              std::string key,
                                              std::vector<xAOD::CaloCluster_t> *container_t,
                                              std::vector<xAOD::CaloCell_t> *container_cells_t,
                                              std::vector<xAOD::CaloDetDescriptor_t> *container_descriptor_t,
                                              bool dump_descriptor) const
{
  SG::ReadHandle<xAOD::CaloClusterContainer> container(key, ctx);

  if (!container.isValid())
  {
    MSG_FATAL("It's not possible to read the xAOD::CaloClusterContainer from this Context using this key " << key);
  }

  std::map<unsigned long int, const xAOD::CaloCell *> cell_map;

  for (const auto clus : **container.ptr())
  {
    xAOD::CaloCluster_t clus_t;
    xAOD::CaloClusterConverter cnv;
    cnv.convert(clus, clus_t);
    container_t->push_back(clus_t);

    if (m_dumpCells)
    {
      serializeCells(clus, container_cells_t, container_descriptor_t, cell_map, dump_descriptor);
    }
  }
}

void RootStreamAODMaker::serializeCells(const xAOD::CaloCluster *clus,
                                        std::vector<xAOD::CaloCell_t> *container_t,
                                        std::vector<xAOD::CaloDetDescriptor_t> *container_descriptor_t,
                                        std::map<unsigned long int, const xAOD::CaloCell *> &cell_map,
                                        bool dump_descriptor) const
{
  for (const auto &cell : clus->cells())
  {
    if (cell_map.count(cell->descriptor()->hash()))
    {
      continue;
    }
    cell_map[cell->descriptor()->hash()] = cell;

    {
      xAOD::CaloCell_t cell_t;
      xAOD::CaloCellConverter cnv;
      cnv.convert(cell, cell_t);
      container_t->push_back(cell_t);
    }

    if (dump_descriptor)
    {
      const xAOD::CaloDetDescriptor *det = cell->descriptor();
      xAOD::CaloDetDescriptor_t det_t;
      xAOD::CaloDetDescriptorConverter cnv;
      cnv.convert(det, det_t);
      container_descriptor_t->push_back(det_t);
    }
  }
}

void RootStreamAODMaker::serializeTruthParticle(EventContext &ctx,
                                                std::string key,
                                                std::vector<xAOD::TruthParticle_t> *container_t) const
{
  SG::ReadHandle<xAOD::TruthParticleContainer> container(key, ctx);

  if (!container.isValid())
  {
    MSG_FATAL("It's not possible to read the xAOD::TruthParticleContainer from this Context using this key " << key);
  }

  for (const auto par : **container.ptr())
  {
    xAOD::TruthParticle_t par_t;
    xAOD::TruthParticleConverter cnv;
    cnv.convert(par, par_t);
    container_t->push_back(par_t);
  }
}

void RootStreamAODMaker::serializeEventInfo(EventContext &ctx,
                                            std::string key,
                                            std::vector<xAOD::EventInfo_t> *container_t) const
{
  SG::ReadHandle<xAOD::EventInfoContainer> event(key, ctx);

  if (!event.isValid())
  {
    MSG_FATAL("It's not possible to read the xAOD::EventInfoContainer from this Context");
  }

  xAOD::EventInfo_t event_t;
  xAOD::EventInfoConverter cnv;
  cnv.convert((**event.ptr()).front(), event_t);
  container_t->push_back(event_t);
}

void RootStreamAODMaker::serializeSeeds(EventContext &ctx,
                                        std::string key,
                                        std::vector<xAOD::Seed_t> *container_t) const
{
  MSG_DEBUG("Serialize Seed..");
  SG::ReadHandle<xAOD::SeedContainer> container(key, ctx);

  if (!container.isValid())
  {
    MSG_FATAL("It's not possible to read the xAOD::SeedContainer from this Context using this key " << key);
  }

  for (const auto seed : **container.ptr())
  {
    xAOD::Seed_t seed_t;
    xAOD::SeedConverter cnv;
    cnv.convert(seed, seed_t);
    container_t->push_back(seed_t);
  }
}

void RootStreamAODMaker::serializeCaloRings(EventContext &ctx,
                                            std::string key,
                                            std::vector<xAOD::CaloRings_t> *container_t) const
{
  MSG_DEBUG("Serialize Rings...");
  SG::ReadHandle<xAOD::CaloRingsContainer> container(key, ctx);

  if (!container.isValid())
  {
    MSG_FATAL("It's not possible to read the xAOD::CaloRingsContainer from this Context using this key " << key);
  }

  for (const auto rings : **container.ptr())
  {
    xAOD::CaloRings_t rings_t;
    xAOD::CaloRingsConverter cnv;
    cnv.convert(rings, rings_t);
    container_t->push_back(rings_t);
  }
}

void RootStreamAODMaker::serializeElectrons(EventContext &ctx,
                                            std::string key,
                                            std::vector<xAOD::Electron_t> *container_t) const
{
  MSG_DEBUG("Serialize Electrons...");
  SG::ReadHandle<xAOD::ElectronContainer> container(key, ctx);
  if (!container.isValid())
  {
    MSG_FATAL("It's not possible to read the xAOD::ElectronContainer using this key " << key);
  }

  for (const auto electron : **container.ptr())
  {
    xAOD::Electron_t electron_t;
    xAOD::ElectronConverter cnv;
    cnv.convert(electron, electron_t);
    container_t->push_back(electron_t);
  }
}

//!=====================================================================
// Metodos aditivos de muon
//!=====================================================================

void RootStreamAODMaker::serializeMuonCells(EventContext &ctx,
                                            std::string key,
                                            std::vector<xAOD::MuonChamber_t> *container_t,
                                            std::vector<xAOD::MuonChamberDescriptor_t> *container_descriptor_t) const
{
  MSG_DEBUG("Serialize Muon Cells...");
  SG::ReadHandle<xAOD::MuonChamberCellContainer> container(key, ctx);

  if (!container.isValid())
  {
    MSG_WARNING("It's not possible to read the xAOD::MuonChamberCellContainer using this key " << key);
    return;
  }

  if ((**container.ptr()).empty())
  {
    MSG_DEBUG("Muon cell container vazio - pulando");
    return;
  }

  xAOD::MuonChamberConverter cnv;
  for (const auto cell : **container.ptr())
  {
    xAOD::MuonChamber_t cell_t;
    cnv.convert(cell, cell_t);
    container_t->push_back(cell_t);

    const xAOD::MuonChamberDescriptor *det = cell->descriptor();
    if (det)
    {
      xAOD::MuonChamberDescriptor_t det_t;
      xAOD::MuonChamberDescriptorConverter desc_cnv;
      desc_cnv.convert(det, det_t);
      container_descriptor_t->push_back(det_t);
    }
  }
}

void RootStreamAODMaker::serializeMuonClusters(EventContext &ctx,
                                               std::string key,
                                               std::vector<xAOD::MuonCluster_t> *container_t) const
{
  MSG_DEBUG("Serialize Muon Clusters...");
  SG::ReadHandle<xAOD::MuonClusterContainer> container(key, ctx);

  if (!container.isValid())
  {
    MSG_WARNING("It's not possible to read the xAOD::MuonClusterContainer using this key " << key);
    return;
  }

  if ((**container.ptr()).empty())
  {
    MSG_DEBUG("Muon cluster container vazio - pulando");
    return;
  }

  xAOD::MuonClusterConverter cnv;
  for (const auto cluster : **container.ptr())
  {
    xAOD::MuonCluster_t cluster_t;
    cnv.convert(cluster, cluster_t);
    container_t->push_back(cluster_t);
  }
}

void RootStreamAODMaker::serializeMuons(EventContext &ctx,
                                        std::string key,
                                        std::vector<xAOD::Muon_t> *container_t) const
{
  MSG_DEBUG("Serialize Muons...");
  SG::ReadHandle<xAOD::MuonContainer> container(key, ctx);

  if (!container.isValid())
  {
    MSG_WARNING("It's not possible to read the xAOD::MuonContainer using this key " << key);
    return;
  }

  if ((**container.ptr()).empty())
  {
    MSG_DEBUG("Muon container vazio - pulando");
    return;
  }

  xAOD::MuonConverter cnv;
  for (const auto muon : **container.ptr())
  {
    xAOD::Muon_t muon_t;
    cnv.convert(muon, muon_t);
    container_t->push_back(muon_t);
  }
}

//!=====================================================================
// serialize() principal: oficial (PRESERVADO) + muon (ADITIVO)
//!=====================================================================

StatusCode RootStreamAODMaker::serialize(EventContext &ctx) const
{
  auto store = ctx.getStoreGateSvc();
  store->cd();
  TTree *tree = store->tree(m_ntupleName);

  // --- Ponteiros dos containers oficiais ---
  std::vector<xAOD::CaloDetDescriptor_t> *container_descriptor = nullptr;
  std::vector<xAOD::CaloCell_t> *container_cells = nullptr;
  std::vector<xAOD::CaloCluster_t> *container_clus = nullptr;
  std::vector<xAOD::CaloRings_t> *container_ringsL0 = nullptr;
  std::vector<xAOD::EventInfo_t> *container_event = nullptr;
  std::vector<xAOD::Seed_t> *container_seeds = nullptr;
  std::vector<xAOD::TruthParticle_t> *container_truth = nullptr;
  std::vector<xAOD::Electron_t> *container_electron = nullptr;
  std::vector<xAOD::CaloCell_t> *container_cells_truth = nullptr;
  std::vector<xAOD::CaloCluster_t> *container_clus_truth = nullptr;
  std::vector<xAOD::Electron_t> *container_electron_truth = nullptr;

  // --- Ponteiros dos containers aditivos de muon ---
  std::vector<xAOD::Muon_t> *container_muon = nullptr;
  std::vector<xAOD::MuonChamber_t> *container_muon_cells = nullptr;
  std::vector<xAOD::MuonChamberDescriptor_t> *container_muon_descriptor = nullptr;
  std::vector<xAOD::MuonCluster_t> *container_muon_clusters = nullptr;

  MSG_DEBUG("Link all branches...");

  // --- Branches oficiais ---
  InitBranch(tree, ("EventInfoContainer_" + m_outputEventKey).c_str(), &container_event);
  InitBranch(tree, ("SeedContainer_" + m_outputSeedsKey).c_str(), &container_seeds);
  InitBranch(tree, ("TruthParticleContainer_" + m_outputTruthKey).c_str(), &container_truth);
  InitBranch(tree, ("CaloRingsContainer_" + m_outputRingerL0Key).c_str(), &container_ringsL0);
  InitBranch(tree, ("CaloClusterContainer_" + m_outputClusterKey).c_str(), &container_clus);
  InitBranch(tree, ("ElectronContainer_" + m_outputElectronKey).c_str(), &container_electron);
  InitBranch(tree, ("CaloClusterContainer_" + m_outputTruthClusterKey).c_str(), &container_clus_truth);
  InitBranch(tree, ("ElectronContainer_" + m_outputTruthElectronKey).c_str(), &container_electron_truth);

  if (m_dumpCells)
  {
    InitBranch(tree, ("CaloCellContainer_" + m_outputCellsKey).c_str(), &container_cells);
    InitBranch(tree, ("CaloDetDescriptorContainer_" + m_outputCellsKey).c_str(), &container_descriptor);
    InitBranch(tree, ("CaloCellContainer_" + m_outputTruthCellsKey).c_str(), &container_cells_truth);
  }

  // --- Branches aditivas de muon ---
  if (m_enableMuonOutput) {
    InitBranch(tree, ("MuonContainer_" + m_outputMuonKey).c_str(), &container_muon);
    if (m_dumpMuonCells) {
      InitBranch(tree, ("MuonChamberCellContainer_" + m_outputMuonCellsKey).c_str(), &container_muon_cells);
      InitBranch(tree, ("MuonChamberDetDescriptorContainer_" + m_outputMuonCellsKey).c_str(), &container_muon_descriptor);
    }
    if (m_dumpMuonClusters) {
      InitBranch(tree, ("MuonClusterContainer_" + m_outputMuonClusterKey).c_str(), &container_muon_clusters);
    }
  }

  // --- Ringer keys (OFICIAL, PRESERVADO) ---
  std::vector<std::vector<xAOD::CaloRings_t> *> containers_rings(m_outputRingerKeys.size());
  for (size_t i = 0; i < m_outputRingerKeys.size(); ++i)
  {
    containers_rings[i] = new std::vector<xAOD::CaloRings_t>();
    InitBranch(tree, ("CaloRingsContainer_" + m_outputRingerKeys[i]).c_str(), &containers_rings[i]);
    if (ctx.exist(m_inputRingerKeys[i]))
      serializeCaloRings(ctx, m_inputRingerKeys[i], containers_rings[i]);
  }

  std::vector<std::vector<xAOD::CaloRings_t> *> containers_rings_truth(m_outputTruthRingerKeys.size());
  for (size_t i = 0; i < m_outputTruthRingerKeys.size(); ++i)
  {
    containers_rings_truth[i] = new std::vector<xAOD::CaloRings_t>();
    InitBranch(tree, ("CaloRingsContainer_" + m_outputTruthRingerKeys[i]).c_str(), &containers_rings_truth[i]);
    if (ctx.exist(m_inputTruthRingerKeys[i]))
      serializeCaloRings(ctx, m_inputTruthRingerKeys[i], containers_rings_truth[i]);
  }

  // --- Serialize oficiais ---
  serializeEventInfo(ctx, m_inputEventKey, container_event);
  serializeSeeds(ctx, m_inputSeedsKey, container_seeds);
  serializeTruthParticle(ctx, m_inputTruthKey, container_truth);

  if (ctx.exist(m_inputClusterKey))
  {
    serializeCaloCluster(ctx, m_inputClusterKey, container_clus, container_cells, container_descriptor, true);
  }
  if (ctx.exist(m_inputRingerL0Key))
  {
    serializeCaloRings(ctx, m_inputRingerL0Key, container_ringsL0);
  }
  if (ctx.exist(m_inputElectronKey))
  {
    serializeElectrons(ctx, m_inputElectronKey, container_electron);
  }
  if (ctx.exist(m_inputTruthClusterKey))
  {
    serializeCaloCluster(ctx, m_inputTruthClusterKey, container_clus_truth, container_cells_truth, container_descriptor, false);
  }
  if (ctx.exist(m_inputTruthElectronKey))
  {
    serializeElectrons(ctx, m_inputTruthElectronKey, container_electron_truth);
  }

  // --- Aditivo: serialize muon ---
  if (m_enableMuonOutput) {
    if (m_dumpMuonCells && container_muon_cells && container_muon_descriptor) {
      if (ctx.exist(m_inputMuonCellsKey))
        serializeMuonCells(ctx, m_inputMuonCellsKey, container_muon_cells, container_muon_descriptor);
    }
    if (m_dumpMuonClusters && container_muon_clusters) {
      if (ctx.exist(m_inputMuonClusterKey))
        serializeMuonClusters(ctx, m_inputMuonClusterKey, container_muon_clusters);
    }
    if (container_muon) {
      if (ctx.exist(m_inputMuonKey))
        serializeMuons(ctx, m_inputMuonKey, container_muon);
    }
  }

  tree->Fill();

  if (m_dumpCells)
  {
    delete container_descriptor;
    delete container_cells;
    delete container_cells_truth;
  }
  delete container_clus;
  delete container_ringsL0;
  delete container_event;
  delete container_truth;
  delete container_seeds;
  delete container_electron;
  delete container_clus_truth;
  delete container_electron_truth;

  // --- Muon ---
  if (container_muon) delete container_muon;
  if (container_muon_cells) delete container_muon_cells;
  if (container_muon_descriptor) delete container_muon_descriptor;
  if (container_muon_clusters) delete container_muon_clusters;

  for (auto *c : containers_rings)
    delete c;
  for (auto *c : containers_rings_truth)
    delete c;

  return StatusCode::SUCCESS;
}
