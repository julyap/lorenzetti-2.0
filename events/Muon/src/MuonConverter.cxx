// ============================================================================
// Arquivo: MuonConverter.cxx
// Descrição: Implementação dos métodos de conversão da classe
//            xAOD::MuonConverter.
//            - convert(const Muon*, Muon_t&): copia todas as variáveis do
//              objeto xAOD::Muon para a struct POD Muon_t, incluindo
//              cinemática, qualidade, isolamento, parâmetros de impacto,
//              flags de working point e variáveis do cluster. O link para o
//              cluster de origem é armazenado como um inteiro (cast do
//              ponteiro) para persistência em ROOT.
//            - convert(const Muon_t&, Muon*&): cria um novo xAOD::Muon a
//              partir da struct Muon_t, preenchendo todas as variáveis
//              correspondentes (cinemática, qualidade, isolamento, impacto,
//              flags e variáveis do cluster).
// ============================================================================

#include "Muon/MuonConverter.h"

using namespace xAOD;

//!=====================================================================
// Serialização: xAOD::Muon* → Muon_t
//   - Copia todas as variáveis primitivas
//   - Copia as flags de working point (VeryLoose/Loose/Medium/Tight)
//   - Copia as variáveis do cluster (fracMax, chi2, q/p sig, rho')
//   - Armazena o ponteiro do cluster como inteiro (cluster_link) para
//     persistência em ROOT; se nullptr, guarda 0
//!=====================================================================
bool MuonConverter::convert( const Muon *muon, Muon_t &muon_t )
{
  if(muon){
    // --- Variáveis cinemáticas ------------------------------------
    muon_t.eta = muon->eta();
    muon_t.phi = muon->phi();
    muon_t.pt  = muon->pt();
    muon_t.e   = muon->e();
    muon_t.et  = muon->et();

    // --- Carga e qualidade ----------------------------------------
    muon_t.charge = muon->charge();
    muon_t.quality = muon->quality();

    // --- Isolamento e parâmetros de impacto -----------------------
    muon_t.iso = muon->isolation();
    muon_t.d0 = muon->d0();
    muon_t.z0 = muon->z0();
    muon_t.sigmaD0 = muon->sigmaD0();
    muon_t.sigmaZ0 = muon->sigmaZ0();

    // --- Flags de working point -----------------------------------
    muon_t.isVeryLoose = muon->isVeryLoose();
    muon_t.isLoose = muon->isLoose();
    muon_t.isMedium = muon->isMedium();
    muon_t.isTight = muon->isTight();

    // --- Variáveis de qualidade do cluster ------------------------
    muon_t.fracMax = muon->fracMax();
    muon_t.chi2 = muon->chi2();
    muon_t.qOverPSignificance = muon->qOverPSignificance();
    muon_t.rhoPrime = muon->rhoPrime();

    // --- Link para o cluster de origem ---------------------------
    //     Armazena o ponteiro como inteiro para persistência em ROOT
    if(muon->muonCluster()) {
      muon_t.cluster_link = reinterpret_cast<unsigned long>(muon->muonCluster());
    } else {
      muon_t.cluster_link = 0;
    }

    return true;
  }
  return false;
}

//!=====================================================================
// Desserialização: Muon_t → xAOD::Muon*
//   - Cria um novo xAOD::Muon a partir de (eta, phi, pt)
//   - Preenche E, ET, carga, qualidade, isolamento e parâmetros de impacto
//   - Restaura as flags de working point
//   - Restaura as variáveis de qualidade do cluster
//!=====================================================================
bool MuonConverter::convert( const Muon_t &muon_t, Muon *&muon)
{
  // Cria o objeto a partir das variáveis cinemáticas básicas
  muon = new xAOD::Muon(muon_t.eta, muon_t.phi, muon_t.pt);

  // --- Preenche variáveis cinemáticas e de impacto ----------------
  muon->setE(muon_t.e);
  muon->setEt(muon_t.et);
  muon->setCharge(muon_t.charge);
  muon->setQuality(muon_t.quality);
  muon->setIsolation(muon_t.iso);
  muon->setD0(muon_t.d0);
  muon->setZ0(muon_t.z0);
  muon->setSigmaD0(muon_t.sigmaD0);
  muon->setSigmaZ0(muon_t.sigmaZ0);

  // --- Restaura as flags de working point -------------------------
  muon->setIsVeryLoose(muon_t.isVeryLoose);
  muon->setIsLoose(muon_t.isLoose);
  muon->setIsMedium(muon_t.isMedium);
  muon->setIsTight(muon_t.isTight);

  // --- Restaura as variáveis de qualidade do cluster -------------
  muon->setFracMax(muon_t.fracMax);
  muon->setChi2(muon_t.chi2);
  muon->setQOverPSignificance(muon_t.qOverPSignificance);
  muon->setRhoPrime(muon_t.rhoPrime);

  return true;
}
