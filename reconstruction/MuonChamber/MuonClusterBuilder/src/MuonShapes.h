// ============================================================================
// Arquivo: MuonShapes.h
// Descrição: Declaração da classe MuonShapes (implementação em
//            MuonShapes.cxx). Define a interface do AlgTool que calcula
//            variáveis de tracking (performTracking, estimatePt,
//            estimateCharge, estimateD0, estimateZ0, fitHelix) e variáveis
//            de qualidade (evaluateLoose/Medium/Tight, calculateChi2,
//            calculateQOverPSignificance, calculateRhoPrime) de um
//            MuonCluster.
// ============================================================================

#ifndef MuonShapes_h
#define MuonShapes_h

#include "GaugiKernel/AlgTool.h"
#include "GaugiKernel/EDM.h"
#include "MuonCluster/MuonClusterContainer.h"
#include "MuonCluster/MuonCluster.h"
#include "MuonCell/MuonChamberCell.h"
#include <vector>
#include <TVector3.h>
#include <TMatrixD.h>
#include <TMatrixDEigen.h>


class MuonShapes : public Gaugi::AlgTool {
public:
  MuonShapes(std::string name);
  ~MuonShapes() = default;

  // --- Ciclo de vida -----------------------------------------------
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode execute(SG::EventContext& ctx, Gaugi::EDM* edm) const override;

private:
  bool m_doForwardMoments;

  // Tracking methods (novos)
  void performTracking(xAOD::MuonCluster* clus) const;
  float estimatePt(const xAOD::MuonCluster* clus) const;
  int estimateCharge(const xAOD::MuonCluster* clus, float pt) const;
  float estimateD0(const xAOD::MuonCluster* clus) const;
  float estimateZ0(const xAOD::MuonCluster* clus) const;
  bool fitHelix(const std::vector<TVector3>& points, float& pt, float& d0, float& z0, float& chi2) const;

  // Métodos de avaliação de qualidade (ATLAS 2016)
  bool evaluateLoose(float fracMax, float chi2, float qOverPSig, float d0, float z0) const;
  bool evaluateMedium(float fracMax, float chi2, float qOverPSig, int nCells, float d0, float z0) const;
  bool evaluateTight(float fracMax, float chi2, float qOverPSig, float rhoPrime, float d0, float z0) const;

  // Métodos auxiliares
  float calculateChi2(const xAOD::MuonCluster* clus) const;
  float calculateQOverPSignificance(const xAOD::MuonCluster* clus) const;
  float calculateRhoPrime(const xAOD::MuonCluster* clus) const;

  // Configuração do tracking
  float m_magneticField;  // Tesla
  int m_minPointsForFit;
  float m_maxD0;
  float m_maxZ0;
};

#endif
