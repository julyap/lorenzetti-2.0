// ============================================================================
// Arquivo: MuonCellMaker.h
// Descrição: Declaração da classe MuonCellMaker (implementação em
//            MuonCellMaker.cxx). Define a interface do algoritmo de
//            reconstrução de células de muon, incluindo métodos de
//            inicialização, execução, histogramas, hooks e funções
//            auxiliares para cálculo de eta/phi/theta e busca em bins.
// ============================================================================

#ifndef MuonCellMaker_h
#define MuonCellMaker_h

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/DataHandle.h"
#include "GaugiKernel/AlgTool.h"


class MuonCellMaker : public Gaugi::Algorithm
{
  public:

    // Construtor
    MuonCellMaker( std::string name );
    ~MuonCellMaker()=default;

    // --- Métodos do ciclo de vida ------------------------------------
    virtual StatusCode initialize() override;
    virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode execute( SG::EventContext &ctx , const G4Step *step) const override;
    virtual StatusCode execute( SG::EventContext &ctx , int /*evt*/ ) const override;
    virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode post_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode finalize() override;

    // Registra Tools e o gerador de pulso
    void push_back( Gaugi::AlgTool *);
    void setPulseGenerator(Gaugi::AlgTool *);

    // Getters de BCID
    int getBcidStart() const { return m_bcid_start; }
    int getBcidEnd() const { return m_bcid_end; }
    int getNBcids() const { return m_bcid_end - m_bcid_start + 1; }

    // NOVOS MÉTODOS AUXILIARES
    float calculateEta(float x, float y, float z) const;
    float calculatePhi(float x, float y) const;
    float calculateTheta(float eta, float z) const;
    bool isEndcapC(float z) const { return z < 0; }
    bool isEndcapA(float z) const { return z > 0; }

  private:
    // Utilitários internos
    std::string getSamplingName(int sampling, int region, int chamberType) const;
    int find( const std::vector<float> &vec, float value) const;
    unsigned long int hash(unsigned bin) const;

    // --- Propriedades configuráveis ---------------------------------
    std::string m_collectionKey;
    std::string m_hitsKey;
    std::string m_histPath;
    int m_layer;
    int m_sampling;
    int m_chamberType;
    int m_region;
    int m_detector;
    int m_bcid_start;
    int m_bcid_end;
    float m_bc_duration;

    std::vector<float> m_etaBins;
    std::vector<float> m_phiBins;
    float m_rMin;
    float m_rMax;
    float m_r;

    bool m_detailedHistograms;

    unsigned int m_nEtaBins;
    unsigned int m_nPhiBins;

    // Ferramentas acopláveis
    Gaugi::AlgTool *m_pulseGenerator;
    std::vector< Gaugi::AlgTool* > m_toolHandles;
};

#endif
