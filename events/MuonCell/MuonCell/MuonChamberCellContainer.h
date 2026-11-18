// ============================================================================
// Arquivo: MuonCell/MuonChamberCellContainer.h
// Autora: Julya Pacheco Ferreira
// Descrição: Define o container padrão de células de câmara de múon
//            (xAOD::MuonChamberCellContainer), que herda de
//            SG::DataVector<xAOD::MuonChamberCell>. Esse container é usado
//            pelo StoreGate para armazenar/recuperar coleções de células
//            de múon.
// ============================================================================

#ifndef MuonChamberCellContainer_h
#define MuonChamberCellContainer_h

#include "MuonCell/MuonChamberCell.h"
#include "GaugiKernel/DataVector.h"

namespace xAOD{
  // Container padrão de células de câmara de múon
  class MuonChamberCellContainer : public SG::DataVector<MuonChamberCell> {
    public:
      MuonChamberCellContainer() = default;
      virtual ~MuonChamberCellContainer() = default;
  };
}

#endif
