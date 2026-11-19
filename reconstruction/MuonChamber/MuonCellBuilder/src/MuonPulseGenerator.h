// ============================================================================
// Arquivo: MuonPulseGenerator.h
// Descrição: Declaração da classe MuonPulseGenerator (implementação em
//            MuonPulseGenerator.cxx). Define a interface do AlgTool que
//            gera o pulso amostrado de uma célula de muon a partir de um
//            template de shaper, ruído e pedestal.
// ============================================================================

#ifndef MuonPulseGenerator_h
#define MuonPulseGenerator_h

#include "GaugiKernel/StatusCode.h"
#include "GaugiKernel/AlgTool.h"
#include "GaugiKernel/EDM.h"
#include "TRandom3.h"


#include "MuonCell/MuonChamberDetDescriptor.h"


namespace xAOD {
  class MuonChamberDescriptor;
}

class MuonPulseGenerator : public Gaugi::AlgTool
{

  public:
    /** Constructor **/
    MuonPulseGenerator( std::string name );
    virtual ~MuonPulseGenerator();

    // --- Ciclo de vida -----------------------------------------------
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Execução por EDM (MuonChamberDescriptor)
    virtual StatusCode execute( SG::EventContext &ctx, Gaugi::EDM * ) const override;

  private:

    // Funções auxiliares
    void ReadShaper( std::string );
    void GenerateDeterministicPulse(  std::vector<float> &pulse,  float amplitude, float phase, float lag) const;
    void AddGaussianNoise( std::vector<float> &pulse, float noiseMean, float noiseStddev) const;


    /*! Number of samples to be generated */
    int m_nsamples;
    int m_startSamplingBC;
    int m_shaperZeroIndex;
    float m_pedestal;
    float m_deformationMean;
    float m_deformationStd;
    float m_samplingRate;
    float m_shaperResolution;
    float m_noiseMean;
    float m_noiseStd;

    std::vector<float> m_shaper;
    std::vector<float> m_timeSeries;

    /*! The shaper configuration path */
    std::string m_shaperFile;
    /*! Output level message */
    int m_outputLevel;
    /*! Random generator */
    mutable TRandom3 m_rng;
};

#endif
