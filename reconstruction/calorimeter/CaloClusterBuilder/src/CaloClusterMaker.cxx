#include "CaloClusterMaker.h"
#include "CaloCell/CaloCell.h"
#include "CaloCluster/CaloCluster.h"
#include "G4Kernel/CaloPhiRange.h"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "TMath.h"

using namespace SG;
using namespace Gaugi;

CaloClusterMaker::CaloClusterMaker(std::string name)
  : Algorithm(),
    m_minCenterEnergy(100. * MeV),
    m_etaWindow(0.2),
    m_phiWindow(0.2),
    m_doForwardMoments(false),
    m_showerShapes(nullptr)
{ }

CaloClusterMaker::~CaloClusterMaker() {}

StatusCode CaloClusterMaker::initialize() {

  return StatusCode::SUCCESS;
}

StatusCode CaloClusterMaker::bookHistograms(EventContext &ctx) const {
  return StatusCode::SUCCESS;
}

StatusCode CaloClusterMaker::pre_execute(EventContext &ctx) const {
  return StatusCode::SUCCESS;
}

StatusCode CaloClusterMaker::execute(EventContext &ctx, const G4Step *step) const {
  return StatusCode::SUCCESS;
}

StatusCode CaloClusterMaker::execute(EventContext &ctx, int evt) const {
  return StatusCode::SUCCESS;
}

StatusCode CaloClusterMaker::fillHistograms(EventContext &ctx) const {
  return StatusCode::SUCCESS;
}

StatusCode CaloClusterMaker::finalize() {
  return StatusCode::SUCCESS;
}

// -----------------------------------------------------------------------------
// Calcular dinamicamente tamanho do cluster em eta
float CaloClusterMaker::calculateClusterDeltaEta(const std::vector<const xAOD::CaloCell*>& cells) const {
  if (cells.empty()) return m_etaWindow / 2.;
  float min_eta = cells.front()->eta();
  float max_eta = cells.front()->eta();
  for (const auto cell : cells) {
    min_eta = std::min(min_eta, cell->eta());
    max_eta = std::max(max_eta, cell->eta());
  }
  return (max_eta - min_eta) * 1.1; // margem de 10%
}

// Calcular dinamicamente tamanho do cluster em phi
float CaloClusterMaker::calculateClusterDeltaPhi(const std::vector<const xAOD::CaloCell*>& cells) const {
  if (cells.empty()) return m_phiWindow / 2.;
  float min_phi = cells.front()->phi();
  float max_phi = cells.front()->phi();
  for (const auto cell : cells) {
    min_phi = std::min(min_phi, cell->phi());
    max_phi = std::max(max_phi, cell->phi());
  }
  return CaloPhiRange::diff(max_phi, min_phi) * 1.1;
}

// -----------------------------------------------------------------------------
StatusCode CaloClusterMaker::post_execute(EventContext &ctx) const {
  SG::WriteHandle<xAOD::CaloClusterContainer> clusters(m_clusterKey, ctx);
  clusters.record(std::unique_ptr<xAOD::CaloClusterContainer>(new xAOD::CaloClusterContainer()));

  SG::ReadHandle<xAOD::SeedContainer> seeds(m_seedKey, ctx);
  SG::ReadHandle<xAOD::CaloCellContainer> container(m_cellsKey, ctx);

  if (!seeds.isValid() || !container.isValid()) {
    MSG_FATAL("Erro ao ler containers");
    return StatusCode::FAILURE;
  }

  for (const auto part : **seeds.ptr()) {
    const xAOD::CaloCell *hotcell = nullptr;
    float emaxs2 = 0.0;

    for (const auto cell : **container.ptr()) {
      const auto* det = cell->descriptor();
      if ((det->sampling() != CaloSampling::EMB2) && (det->sampling() != CaloSampling::EMEC2)) continue;

      float deltaEta = std::abs(part->eta() - cell->eta());
      float deltaPhi = std::abs(CaloPhiRange::diff(part->phi(), cell->phi()));

      if (deltaEta < m_etaWindow / 2 && deltaPhi < m_phiWindow / 2 && cell->e() > emaxs2) {
        hotcell = cell;
        emaxs2 = cell->e();
      }
    }

    if (!hotcell) {
      MSG_DEBUG("Sem hotcell para essa seed.");
      continue;
    }

    float etot = 0.0;
    std::vector<const xAOD::CaloCell*> clusterCells;

    for (const auto cell : **container.ptr()) {
      if (cell->descriptor()->detector() != Detector::TTEM) continue;

      float deltaEta = std::abs(hotcell->eta() - cell->eta());
      float deltaPhi = std::abs(CaloPhiRange::diff(hotcell->phi(), cell->phi()));

      if (deltaEta < 0.05 && deltaPhi < 0.05) {
        etot += cell->e();
      }

      if (deltaEta < m_etaWindow / 2 && deltaPhi < m_phiWindow / 2) {
        clusterCells.push_back(cell);
      }
    }

    MSG_INFO("Energia central EM: " << etot);

    if (etot < m_minCenterEnergy) continue;

    float deta = calculateClusterDeltaEta(clusterCells);
    float dphi = calculateClusterDeltaPhi(clusterCells);

    xAOD::CaloCluster* clus = new xAOD::CaloCluster(hotcell->e(), hotcell->eta(), hotcell->phi(), deta, dphi);
    clus->setSeed(part);

    fillCluster(ctx, clus, m_cellsKey);
    m_showerShapes->execute(ctx, clus);

    clusters->push_back(clus);
  }

  return StatusCode::SUCCESS;
}

