#ifndef Evtgen_Jpsi_h
#define Evtgen_Jpsi_h

#include "GenKernel/IAlgorithm.h"
#include "GenKernel/IGenerator.h"

namespace generator {

  class Jpsi : public IAlgorithm
  {
    public:
    
      Jpsi(const std::string& name, IGenerator* gen);
      ~Jpsi();
  
      virtual StatusCode initialize() override;
      virtual StatusCode execute(Event& ctx) override;
      virtual StatusCode finalize() override;
  
    protected:  // Mudado de private para protected
      float m_minPt;
      float m_etaMax;
      bool m_zeroVertexParticles;
      
      // Helper para verificar se é um lépton válido (múon ou elétron)
      bool isValidLepton(int pdgId);
      
      // Helper para obter o nome do lépton
      const char* getLeptonName(int pdgId);
  };

}
#endif
