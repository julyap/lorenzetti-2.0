// ============================================================================
// Arquivo: Muon.h
// Descrição: Declaração da classe xAOD::Muon, que representa um múon
//            reconstruído no EDM do ATLAS. Herda de Gaugi::EDM e expõe
//            getters/setters para as variáveis cinemáticas (η, φ, E, ET, pT,
//            carga), de qualidade (quality, isolamento, d0, z0, σ(d0),
//            σ(z0)) e do cluster associado (fracMax, chi2, q/p significance,
//            rho'). Também fornece métodos auxiliares para verificar se o
//            múon passa nos working points (VeryLoose/Loose/Medium/Tight) e
//            se está na região forward (|η| > 2.5).
// ============================================================================

#ifndef Muon_h
#define Muon_h

#include "MuonCluster/MuonCluster.h"
#include "GaugiKernel/EDM.h"
#include "GaugiKernel/macros.h"
#include <cmath>

namespace xAOD{

  class Muon: public Gaugi::EDM
  {
    public:

      /*! Constructor */
      Muon();
      /*! Constructor com parâmetros */
      Muon(float eta, float phi, float pt);
      /*! Destructor */
      ~Muon() = default;

      /*! Getters and Setters */
      // --- Variáveis cinemáticas ------------------------------------
      PRIMITIVE_SETTER_AND_GETTER( float, m_eta, setEta, eta );
      PRIMITIVE_SETTER_AND_GETTER( float, m_phi, setPhi, phi );
      PRIMITIVE_SETTER_AND_GETTER( float, m_e, setE, e );
      PRIMITIVE_SETTER_AND_GETTER( float, m_et, setEt, et );
      PRIMITIVE_SETTER_AND_GETTER( float, m_pt, setPt, pt );

      // --- Carga e qualidade ----------------------------------------
      PRIMITIVE_SETTER_AND_GETTER( int, m_charge, setCharge, charge );
      PRIMITIVE_SETTER_AND_GETTER( int, m_quality, setQuality, quality );

      // --- Isolamento e parâmetros de impacto -----------------------
      PRIMITIVE_SETTER_AND_GETTER( float, m_iso, setIsolation, isolation );
      PRIMITIVE_SETTER_AND_GETTER( float, m_d0, setD0, d0 );
      PRIMITIVE_SETTER_AND_GETTER( float, m_z0, setZ0, z0 );
      PRIMITIVE_SETTER_AND_GETTER( float, m_sigmaD0, setSigmaD0, sigmaD0 );
      PRIMITIVE_SETTER_AND_GETTER( float, m_sigmaZ0, setSigmaZ0, sigmaZ0 );

      /*! NOVOS MÉTODOS - Variáveis de qualidade do cluster */
      // --- Variáveis de qualidade herdadas do cluster ---------------
      PRIMITIVE_SETTER_AND_GETTER( float, m_fracMax, setFracMax, fracMax );
      PRIMITIVE_SETTER_AND_GETTER( float, m_chi2, setChi2, chi2 );
      PRIMITIVE_SETTER_AND_GETTER( float, m_qOverPSignificance, setQOverPSignificance, qOverPSignificance );
      PRIMITIVE_SETTER_AND_GETTER( float, m_rhoPrime, setRhoPrime, rhoPrime );

      /*! Set/Get muon cluster */
      // --- Referência ao cluster de origem -------------------------
      void setMuonCluster( const xAOD::MuonCluster *cluster ){ m_muonCluster = cluster; };
      const xAOD::MuonCluster* muonCluster() const { return m_muonCluster; };

      /*! Quality flags (like Electron) */
      // --- Verificação dos working points (baseada em m_quality) ----
      bool isVeryLoose() const { return m_quality >= 1; };
      bool isLoose() const { return m_quality >= 2; };
      bool isMedium() const { return m_quality >= 3; };
      bool isTight() const { return m_quality >= 4; };

      // --- Atualização dos working points (só aumenta a qualidade) --
      void setIsVeryLoose(bool isVeryLoose) { if(isVeryLoose && m_quality < 1) m_quality = 1; };
      void setIsLoose(bool isLoose) { if(isLoose && m_quality < 2) m_quality = 2; };
      void setIsMedium(bool isMedium) { if(isMedium && m_quality < 3) m_quality = 3; };
      void setIsTight(bool isTight) { if(isTight && m_quality < 4) m_quality = 4; };

      /*! Check if muon is in forward region */
      // --- Região forward: |η| > 2.5 -------------------------------
      bool isForward() const { return std::abs(m_eta) > 2.5; };

    private:
      // --- Variáveis cinemáticas ------------------------------------
      float m_e{0};
      float m_et{0};
      float m_pt{0};
      float m_eta{0};
      float m_phi{0};
      int m_charge{0};

      // --- Qualidade e isolamento -----------------------------------
      int m_quality{0};
      float m_iso{0};

      // --- Parâmetros de impacto ------------------------------------
      float m_d0{0};
      float m_z0{0};
      float m_sigmaD0{0};
      float m_sigmaZ0{0};

      /*! NOVAS VARIÁVEIS - Qualidade do cluster */
      float m_fracMax{0};
      float m_chi2{0};
      float m_qOverPSignificance{0};
      float m_rhoPrime{0};

      // --- Referência ao cluster de origem --------------------------
      const xAOD::MuonCluster *m_muonCluster{nullptr};
  };
}

#endif
