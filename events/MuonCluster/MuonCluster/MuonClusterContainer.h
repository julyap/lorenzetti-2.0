// ============================================================================
// Arquivo: MuonCluster/MuonCluster/MuonClusterContainer.h
// Autora: Julya Pacheco Ferreira
// Descrição: Define o typedef xAOD::MuonClusterContainer como um
//            SG::DataVector<xAOD::MuonCluster>, ou seja, um container padrão
//            de clusters de múon do EDM. Esse typedef é usado pelo StoreGate
//            para armazenar/recuperar coleções de clusters.
// ============================================================================

#ifndef MuonClusterContainer_h
#define MuonClusterContainer_h

/** simulator includes **/
#include "MuonCluster/MuonCluster.h"
#include "GaugiKernel/DataVector.h"

namespace xAOD{
  // Container padrão de clusters de múon (usado pelo StoreGate)
  typedef SG::DataVector<xAOD::MuonCluster> MuonClusterContainer;
}
#endif
