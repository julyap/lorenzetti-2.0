// ============================================================================
// Arquivo: MuonHitMaker.h
// Autora: Julya Pacheco Ferreira
// Descrição: Declaração da classe MuonHitMaker (implementação em
//            MuonHitMaker.cxx). Define a interface do algoritmo de criação
//            de hits de muon a partir dos steps do Geant4 e dos métodos
//            auxiliares (find, hash, determineRegionFromZ).
// ============================================================================

#ifndef MuonHitMaker_h
#define MuonHitMaker_h

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/DataHandle.h"
#include "G4UserSteppingAction.hh"


class MuonHitMaker : public Gaugi::Algorithm
{
  public:
    MuonHitMaker( std::string name );
    virtual ~MuonHitMaker()=default;

    // --- Ciclo de vida -----------------------------------------------
    virtual StatusCode initialize() override;
    virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode execute( SG::EventContext &ctx , const G4Step *step ) const override;
    virtual StatusCode execute( SG::EventContext &ctx , int evt ) const override;
    virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode post_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode finalize() override;

    // NOVOS MÉTODOS DECLARADOS AQUI!
    int determineRegionFromZ(float z_mm) const;

  private:
    // Utilitários internos
    int find( const std::vector<float> &vec, float value) const;
    unsigned long int hash(unsigned bin) const;

    // --- Propriedades configuráveis ---------------------------------
    std::string m_collectionKey;
    std::vector<float> m_xBins;
    std::vector<float> m_yBins;
    std::vector<float> m_zBins;
    float m_rMin;
    float m_rMax;
    float m_zMin;
    float m_zMax;
    int m_sampling;
    int m_detector;
    int m_region;
    int m_bcid_start;
    int m_bcid_end;
    float m_bc_duration;
    int m_outputLevel;
    bool m_detailedHistograms;
    std::string m_histPath;
    float m_noiseStd;
    bool m_useCartesianGrid;

    unsigned int m_nXBins;
    unsigned int m_nYBins;
    unsigned int m_nZBins;
};

#endif
