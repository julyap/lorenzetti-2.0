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
#include "MuonHit/MuonHitContainer.h"
#include "EventInfo/EventInfoContainer.h"
#include "TruthParticle/TruthParticleContainer.h"

#include "TTree.h"
#include "TFile.h"

namespace Gaugi {

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

  // Configurable properties
  std::string m_inputFile;
  std::string m_eventKey;
  std::string m_truthKey;
  std::string m_caloHitsKey;   // Chave específica para CaloHits
  std::string m_muonHitsKey;   // Chave específica para MuonHits
  std::string m_seedsKey;
  
  std::string m_ntupleName;
  int m_outputLevel;
  bool m_enableMuonInput;
};

} // namespace Gaugi

#endif // ROOTSTREAMHITREADER_H
