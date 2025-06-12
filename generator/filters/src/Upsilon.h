#ifndef Evtgen_Upsilon_h
#define Evtgen_Upsilon_h

#include "GenKernel/IAlgorithm.h"
#include "GenKernel/IGenerator.h"

namespace generator {

  class Upsilon : public IAlgorithm
  {
    public:
    
      Upsilon(const std::string& name, IGenerator* gen);  // Ajuste aqui para o tipo correto
      ~Upsilon();
  
      virtual StatusCode initialize() override;
      virtual StatusCode execute(Event& ctx) override;
      virtual StatusCode finalize() override;
  
    private:
      float m_minPt;
      float m_etaMax;
      bool m_zeroVertexParticles;
      bool m_forceForwardUpsilon; // Corrigido para coincidir com o membro
  };

}
#endif

