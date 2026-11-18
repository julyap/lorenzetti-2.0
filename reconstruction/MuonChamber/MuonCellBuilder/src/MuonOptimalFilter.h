// ============================================================================
// Arquivo: MuonOptimalFilter.h
// Autora: Julya Pacheco Ferreira
// Descrição: Declaração da classe MuonOptimalFilter (implementação em
//            MuonOptimalFilter.cxx). Define a interface do AlgTool que
//            aplica o filtro ótimo (pesos de energia e tempo) a um
//            MuonChamberDescriptor, atribuindo a energia reconstruída.
// ============================================================================

#ifndef MuonOptimalFilter_h
#define MuonOptimalFilter_h

#include "GaugiKernel/AlgTool.h"
#include "GaugiKernel/EDM.h"


class MuonOptimalFilter : public Gaugi::AlgTool
{

  public:
    /** Constructor **/
    MuonOptimalFilter( std::string name );
    virtual ~MuonOptimalFilter();

    // --- Ciclo de vida -----------------------------------------------
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Execução por EDM (MuonChamberDescriptor)
    virtual StatusCode execute( SG::EventContext &ctx, Gaugi::EDM * ) const override;

  private:

    /*! optimal filter weights */
    std::vector<float> m_ofweightsEnergy;   // pesos de energia por amostra
    std::vector<float> m_ofweightsTime;     // pesos de tempo por amostra
};

#endif
