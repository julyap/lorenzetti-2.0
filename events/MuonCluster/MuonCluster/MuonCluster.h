// ============================================================================
// Arquivo: MuonCluster/MuonCluster/MuonCluster.h
// Descrição: Declaração da classe xAOD::MuonCluster, que representa um
//            cluster de células de múon no EDM do ATLAS. Herda de
//            Gaugi::EDM e armazena:
//              - Propriedades básicas: emaxs2, eta, phi, deta, dphi e seed;
//              - Energia: totalEnergy, fracMax;
//              - Qualidade da track: chi2, qOverPSignificance, rhoPrime;
//              - Flags de qualidade: isVeryLoose, isLoose, isMedium, isTight
//                e um código inteiro (quality);
//              - Variáveis de tracking: charge, pt, p, d0, z0, trackChi2;
//              - Número de camadas com hits (nLayers);
//              - Container de ponteiros para as células que compõem o
//                cluster (m_container).
//            Também fornece métodos auxiliares de região (isForward,
//            isBarrel, isEndcap, isCentralGap).
// ============================================================================

#ifndef MUONCLUSTER_H
#define MUONCLUSTER_H

#include "GaugiKernel/EDM.h"
#include "MuonCell/MuonChamberCell.h"
#include "TVector3.h"
#include <vector>

namespace xAOD {

class MuonCluster : public Gaugi::EDM
{
public:
    /** Constructors **/
    MuonCluster();
    MuonCluster(float emaxs2, float eta, float phi, float deta, float dphi);

    /** Destructor **/
    ~MuonCluster();

    /** Clear all cells **/
    void clear();

    /** Add a MuonChamberCell into the cluster **/
    void push_back(const xAOD::MuonChamberCell* cell);

    /** Number of cells **/
    size_t size() const;

    /** Get all cells **/
    const std::vector<const xAOD::MuonChamberCell*>& cells() const;

    /** Setters and Getters for basic properties **/
    // --- Propriedades básicas do cluster --------------------------
    void setEmaxs2(float e) { m_emaxs2 = e; }
    float emaxs2() const { return m_emaxs2; }

    void setEta(float eta) { m_eta = eta; }
    float eta() const { return m_eta; }

    void setPhi(float phi) { m_phi = phi; }
    float phi() const { return m_phi; }

    void setDeta(float deta) { m_deta = deta; }
    float deta() const { return m_deta; }

    void setDphi(float dphi) { m_dphi = dphi; }
    float dphi() const { return m_dphi; }

    // --- Célula semente (seed) do cluster -------------------------
    void setSeed(const xAOD::MuonChamberCell* seed) { m_seed = seed; }
    const xAOD::MuonChamberCell* seed() const { return m_seed; }



// Tracking variables
    // --- Variáveis de tracking ------------------------------------
    void setCharge(int q) { m_charge = q; }
    int charge() const { return m_charge; }

    void setPt(float pt) { m_pt = pt; }
    float pt() const { return m_pt; }

    void setP(float p) { m_p = p; }
    float p() const { return m_p; }

    void setD0(float d0) { m_d0 = d0; }
    float d0() const { return m_d0; }

    void setZ0(float z0) { m_z0 = z0; }
    float z0() const { return m_z0; }

    void setTrackChi2(float chi2) { m_trackChi2 = chi2; }
    float trackChi2() const { return m_trackChi2; }




    /** Energy methods **/
    // --- Energia ---------------------------------------------------
    void setTotalEnergy(float e) { m_totalEnergy = e; }
    float totalEnergy() const { return m_totalEnergy; }
    float energy() const { return m_totalEnergy; }

    void setFracMax(float fm) { m_fracMax = fm; }
    float fracMax() const { return m_fracMax; }

    /** Track quality variables **/
    // --- Variáveis de qualidade da track --------------------------
    void setChi2(float c) { m_chi2 = c; }
    float chi2() const { return m_chi2; }

    void setQOverPSignificance(float q) { m_qOverPSignificance = q; }
    float qOverPSignificance() const { return m_qOverPSignificance; }

    void setRhoPrime(float r) { m_rhoPrime = r; }
    float rhoPrime() const { return m_rhoPrime; }

    /** Muon quality flags **/
    // --- Flags de qualidade (working points) ----------------------
    void setIsVeryLoose(bool v) { m_isVeryLoose = v; }
    bool isVeryLoose() const { return m_isVeryLoose; }

    void setIsLoose(bool v) { m_isLoose = v; }
    bool isLoose() const { return m_isLoose; }

    void setIsMedium(bool v) { m_isMedium = v; }
    bool isMedium() const { return m_isMedium; }

    void setIsTight(bool v) { m_isTight = v; }
    bool isTight() const { return m_isTight; }

    void setQuality(int q) { m_quality = q; }
    int quality() const { return m_quality; }

    // Uma camada MDT típica tem 6-8 células (tubos)
    int nLayers() const { return m_nLayers; }
    void setNLayers(int n) { m_nLayers = n; }

    // NOVO: Identificação da região
    // --- Identificação de região ----------------------------------
    bool isForward() const { return std::abs(m_eta) > 2.5; }
    bool isBarrel() const { return std::abs(m_eta) < 1.05; }
    bool isEndcap() const { return std::abs(m_eta) >= 1.05 && std::abs(m_eta) <= 2.5; }
    bool isCentralGap() const { return std::abs(m_eta) < 0.1; }


private:
    // Basic properties
    float m_emaxs2;
    float m_eta;
    float m_phi;
    float m_deta;
    float m_dphi;
    const xAOD::MuonChamberCell* m_seed;

    // Energy information
    float m_totalEnergy;
    float m_fracMax;

    // Track quality variables
    float m_chi2;
    float m_qOverPSignificance;
    float m_rhoPrime;

    // Muon quality flags
    bool m_isVeryLoose;
    bool m_isLoose;
    bool m_isMedium;
    bool m_isTight;
    int m_quality;

    // No private section:
    // --- Variáveis de tracking ------------------------------------
    int m_charge;
    float m_pt;
    float m_p;
    float m_d0;
    float m_z0;
    float m_trackChi2;
    int m_nLayers{0};  // Número de camadas MS com hits


    // Cells container
    // --- Container de células que compõem o cluster ---------------
    std::vector<const xAOD::MuonChamberCell*> m_container;
};

} // namespace xAOD

#endif // MUONCLUSTER_H
