// ============================================================================
// Arquivo: MuonContainer.h
// Descrição: Define o typedef xAOD::MuonContainer como um
//            SG::DataVector<xAOD::Muon>. Esse typedef é usado pelo
//            StoreGate para armazenar/recuperar coleções de múons.
// ============================================================================

#ifndef MuonContainer_h
#define MuonContainer_h

#include "GaugiKernel/DataVector.h"
#include "Muon/Muon.h"

namespace xAOD {
  // Container padrão de múons reconstruídos (usado pelo StoreGate)
  typedef SG::DataVector<xAOD::Muon> MuonContainer;
}

#endif
