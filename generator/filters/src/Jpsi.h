#ifndef Evtgen_Jpsi_h
#define Evtgen_Jpsi_h

#include "GenKernel/IAlgorithm.h"
#include "GenKernel/IGenerator.h"

namespace generator {

  class Jpsi : public IAlgorithm
  {
    public:
    
      Jpsi(const std::string& name, IGenerator* gen);  // Ajuste aqui para o tipo correto
      ~Jpsi();
  
      virtual StatusCode initialize() override;
      virtual StatusCode execute(Event& ctx) override;
      virtual StatusCode finalize() override;
  
    private:
      float m_minPt;
      float m_etaMax;
      bool m_zeroVertexParticles;
      bool m_forceForwardJpsi; // Corrigido para coincidir com o membro
  };

}
#endif

