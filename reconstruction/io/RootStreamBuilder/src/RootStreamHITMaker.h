#ifndef ROOTSTREAMHITMAKER_H
#define ROOTSTREAMHITMAKER_H

#include <string>
#include <vector>

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/DataHandle.h"
#include "GaugiKernel/StatusCode.h"
#include "GaugiKernel/StoreGate.h"
#include "GaugiKernel/macros.h"

#include "CaloHit/CaloHitContainer.h"
#include "MuonHit/MuonHitContainer.h"
#include "EventInfo/EventInfoContainer.h"
#include "TruthParticle/TruthParticleContainer.h"
#include "TTree.h"

namespace Gaugi {

/**
 * @class RootStreamHITMaker
 * @brief Algorithm to stream Geant4 Hits to a ROOT file.
 * 
 * Validates and writes energy deposits (Hits), truth particles, and seeds
 * to a persistent file. This output serves as input for the Digitization step.
 *
 * --- Aditivo: muon ---
 * Also writes muon chamber hits (MuonHitContainer) when EnableMuonOutput is true.
 */
class RootStreamHITMaker : public Algorithm
{
public:
  explicit RootStreamHITMaker(std::string name);
  virtual ~RootStreamHITMaker();

  virtual StatusCode initialize() override;
  virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;
  virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;
  virtual StatusCode execute( SG::EventContext &ctx, const G4Step* step ) const override;
  virtual StatusCode execute( SG::EventContext &ctx, int evt ) const override;
  virtual StatusCode post_execute( SG::EventContext &ctx ) const override;
  virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;
  virtual StatusCode finalize() override;

  // Funções auxiliares para identificação de partículas (aditivas)
  static bool isElectron(int pdgId);
  static bool isMuon(int pdgId);
  static bool isPhoton(int pdgId);
  static bool isHadron(int pdgId);

private:

  StatusCode serialize( SG::EventContext &ctx ) const;

  // Função auxiliar para verificar se hit está em RoI (aditiva)
  bool isHitInRoI(float hit_eta, float hit_phi, 
                  const std::vector<float>& particle_etas,
                  const std::vector<float>& particle_phis,
                  float etaWindow, float phiWindow) const;

  template <class T>
  void InitBranch(TTree* fChain, std::string branch_name, T* param) const;

  // --- Oficiais: calorimetro ---
  std::string m_inputHitsKey;
  std::string m_outputHitsKey;
  std::vector<int> m_cellHashes;   // KeepCells

  // --- Aliases da sua versao ---
  std::string m_inputCaloHitsKey;
  std::string m_outputCaloHitsKey;

  // --- Aditivo: muon ---
  std::string m_inputMuonHitsKey;
  std::string m_outputMuonHitsKey;
  float m_muonEtaWindow;
  float m_muonPhiWindow;
  bool m_enableMuonOutput;

  // --- Oficiais (comuns) ---
  std::string m_inputEventKey;
  std::string m_inputTruthKey;
  std::string m_inputSeedsKey;

  std::string m_outputEventKey;
  std::string m_outputTruthKey;
  std::string m_outputSeedsKey;
  
  int m_outputLevel;
  std::string m_ntupleName;
  bool m_onlyRoI;
  float m_etaWindow;
  float m_phiWindow;
};

} // namespace Gaugi

#endif // ROOTSTREAMHITMAKER_H
