// ============================================================================
// Arquivo: MuonClusterMaker.h
// Descrição: Declaração da classe MuonClusterMaker (implementação em
//            MuonClusterMaker.cxx). Define a interface do algoritmo de
//            clustering de células de muon, incluindo as funções de
//            avaliação de qualidade (Loose/Medium/Tight), cálculo de chi2,
//            q/p significance e rho', e a função de distância em (eta, phi).
// ============================================================================

#ifndef MUONCLUSTERMAKER_H
#define MUONCLUSTERMAKER_H

#include "GaugiKernel/Algorithm.h"
#include "G4Step.hh"
#include "MuonCluster/MuonClusterContainer.h"
#include "MuonCell/MuonChamberCellContainer.h"

namespace Gaugi {
  class StoreGate;
}


class MuonClusterMaker : public Gaugi::Algorithm
{
public:
    MuonClusterMaker( std::string name="MuonClusterMaker" );
    ~MuonClusterMaker();

    // --- Ciclo de vida -----------------------------------------------
    virtual StatusCode initialize() override;
    virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode execute( SG::EventContext &ctx , const G4Step *step ) const override;
    virtual StatusCode execute( SG::EventContext &ctx , int evt ) const override;
    virtual StatusCode post_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode finalize() override;


private:
    // Evaluation functions for muon quality
    bool evaluateLoose(float fracMax, float chi2, float qOverPSig) const;
    bool evaluateMedium(float fracMax, float chi2, float qOverPSig, int nCells) const;
    bool evaluateTight(float fracMax, float chi2, float qOverPSig, float rhoPrime) const;

    // Calculation functions
    float calculateChi2(const xAOD::MuonCluster* clus, float totalEnergy) const;
    float calculateQOverPSignificance(const xAOD::MuonCluster* clus, float fracMax) const;
    float calculateRhoPrime(const xAOD::MuonCluster* clus) const;

    void fillCluster( SG::EventContext &ctx, xAOD::MuonCluster *clus, std::string key ) const;
    float dR( float eta1, float phi1, float eta2, float phi2 ) const;

    // Properties (configuráveis)
    std::string m_cellsKey;
    std::string m_seedsKey;
    std::string m_hitsKey;
    std::string m_clusterKey;

    float m_etaWindow;              // janela em eta
    float m_phiWindow;              // janela em phi
    float m_xWindow;                // janela em X
    float m_yWindow;                // janela em Y
    float m_zWindow;                // janela em Z
    float m_maxSeedDistance;        // distância máxima à semente
    float m_rWindow;                // janela radial
    int m_minCellsPerCluster;       // mínimo de células por cluster
    bool m_useEnergyCentroid;       
    bool m_calculateShapes;         
    bool m_doForwardMoments;       
    std::string m_histPath;         // caminho dos histogramas
    float m_minCenterEnergy;        // energia mínima da semente


};

#endif // MUONCLUSTERMAKER_H
