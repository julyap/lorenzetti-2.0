#ifndef ROOTSTREAMHITREADER_H
#define ROOTSTREAMHITREADER_H

#include <string>
#include <vector>

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/DataHandle.h"
#include "GaugiKernel/StatusCode.h"
#include "GaugiKernel/StoreGate.h"
#include "GaugiKernel/macros.h"

#include "CaloHit/CaloHitContainer.h"
#include "EventInfo/EventInfoContainer.h"
#include "TruthParticle/TruthParticleContainer.h"

#include "TTree.h"
#include "TFile.h"



namespace Gaugi {

/**
 * @class RootStreamHITReader
 * @brief Algorithm to read Hit-level data from a ROOT file.
 *
 * deserializes Hits, TruthParticles, and Seeds from a persistent ROOT format
 * back into the transient Event Store (StoreGate) for further processing (e.g., Digitization).
 *
 * --- Aditivo: muon ---
 * Also reads muon chamber hits (MuonHitContainer) when EnableMuonInput is true.
 */
class RootStreamHITReader : public Algorithm
{
public:
  explicit RootStreamHITReader(std::string name);
  virtual ~RootStreamHITReader();

  virtual StatusCode initialize() override;
  virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;
  virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;
  virtual StatusCode execute( SG::EventContext &ctx, const G4Step* step ) const override;
  virtual StatusCode execute( SG::EventContext &ctx, int evt ) const override;
  virtual StatusCode post_execute( SG::EventContext &ctx ) const override;
  virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;
  virtual StatusCode finalize() override;

private:

  StatusCode deserialize( int evt, SG::EventContext &ctx ) const;

  template <class T>
  void InitBranch(TTree* fChain, std::string branch_name, T* param) const;

  // --- Oficiais: calorimetro ---
  std::string m_hitsKey;
  std::string m_caloHitsKey;

  // --- Aditivo: muon ---
  std::string m_muonHitsKey;
  bool m_enableMuonInput;

  // --- Oficiais (comuns) ---
  std::string m_inputFile;
  std::string m_eventKey;
  std::string m_truthKey;
  std::string m_seedsKey;

  std::string m_ntupleName;
  int m_outputLevel;
};

} // namespace Gaugi

#endif // ROOTSTREAMHITREADER_H
