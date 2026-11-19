#ifndef ROOTSTREAMNTUPLEMAKER_H
#define ROOTSTREAMNTUPLEMAKER_H

#include "GaugiKernel/Algorithm.h"
#include "CaloCell/enumeration.h"
#include "EventInfo/EventInfo.h"
#include "TruthParticle/TruthParticle.h"
#include "CaloCluster/CaloCluster.h"
#include "CaloRings/CaloRings.h"
#include "Egamma/Electron.h"
#include "TTree.h"
#include <vector>
#include <string>

/**
 * @class RootStreamNtupleMaker
 * @brief Algorithm to dump analysis-level variables into a flat Ntuple (TTree).
 *
 * --- Aditivo: muon ---
 * - EnableCaloOutput=false skips all calorimeter/electron reads and the
 *   per-electron loop, allowing standalone muon-only ntuples.
 * - EnableMuonOutput=true adds flat muon branches (mu_n, mu_pt, mu_eta, ...).
 * - In standalone muon mode, one tree entry is written per event.
 */
class RootStreamNtupleMaker : public Gaugi::Algorithm
{

public:
  RootStreamNtupleMaker(std::string);
  virtual ~RootStreamNtupleMaker();

  virtual StatusCode initialize() override;
  virtual StatusCode bookHistograms(SG::EventContext &ctx) const override;
  virtual StatusCode pre_execute(SG::EventContext &ctx) const override;
  virtual StatusCode execute(SG::EventContext &ctx, const G4Step *step) const override;
  virtual StatusCode execute(SG::EventContext &ctx, int evt) const override;
  virtual StatusCode post_execute(SG::EventContext &ctx) const override;
  virtual StatusCode fillHistograms(SG::EventContext &ctx) const override;
  virtual StatusCode finalize() override;

private:
  template <class T>
  void InitBranch(TTree *fChain, std::string branch_name, T *param) const;

  // --- Oficiais: calorimetro ---
  std::string m_seedsKey;
  std::string m_eventKey;
  std::string m_truthKey;
  std::string m_clusterKey;
  std::string m_ringerKey;
  std::string m_electronKey;
  std::string m_ringerL0Key;
  std::string m_truthClusterKey;
  std::string m_truthRingerKey;
  std::string m_truthElectronKey;
  std::string m_outputNtupleName;

  std::vector<std::string> m_ringerKeys;
  std::vector<std::string> m_truthRingerKeys;

  // --- Aditivo: muon ---
  std::string m_muonKey;
  std::string m_muonClusterKey;       // (reservado; usado pelo ModeMuonCells se habilitado)
  bool m_enableCaloOutput;            // default true
  bool m_enableMuonOutput;            // default true

  int m_outputLevel;
};

#endif
