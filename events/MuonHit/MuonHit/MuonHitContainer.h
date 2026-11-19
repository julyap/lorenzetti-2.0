// ============================================================================
// Arquivo: MuonHit/MuonHit/MuonHitContainer.h
// Descrição: Define o typedef xAOD::MuonHitContainer como um
//            SG::DataVector<xAOD::MuonHit>, ou seja, um container padrão de
//            hits de múon do EDM. Usado pelo StoreGate para armazenar e
//            recuperar coleções de hits em memória.
// ============================================================================

#ifndef MuonHitContainer_h
#define MuonHitContainer_h

/** includes do simulador **/
#include "MuonHit/MuonHit.h"          // Classe de hit de múon adaptada
#include "GaugiKernel/DataVector.h"   // Template DataVector da framework

namespace xAOD{
  // Todos os objetos dentro do data vector serão const
  // Define um container especializado para hits de múon
  typedef SG::DataVector<xAOD::MuonHit> MuonHitContainer;
}
#endif
