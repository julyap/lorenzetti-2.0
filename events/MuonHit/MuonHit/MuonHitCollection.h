// ============================================================================
// Arquivo: MuonHit/MuonHit/MuonHitCollection.h
// Descrição: Define o typedef xAOD::MuonHitCollection como um
//            SG::Collection<xAOD::MuonHit>, ou seja, uma coleção
//            especializada de hits de múon que armazena apenas objetos
//            const. Usada pelo StoreGate para armazenar/recuperar coleções
//            de hits.
// ============================================================================

#ifndef MuonHitCollection_h
#define MuonHitCollection_h

/** includes do simulador **/
#include "MuonHit/MuonHit.h"          // Classe de hit de múon adaptada
#include "GaugiKernel/Collection.h"   // Template de coleção da framework

namespace xAOD{
  // Todos os objetos dentro do vetor de dados serão const
  // Define uma coleção especializada para hits de múon
  typedef SG::Collection<xAOD::MuonHit> MuonHitCollection;
}
#endif
