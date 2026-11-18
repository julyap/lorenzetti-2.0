// ============================================================================
// Arquivo: MuonCell/MuonChamberDetDescriptorCollection.h
// Autora: Julya Pacheco Ferreira
// Descrição: Define o container padrão de descriptors de câmara de múon
//            (xAOD::MuonChamberDetDescriptorCollection), que herda de
//            SG::Collection<xAOD::MuonChamberDescriptor>. Esse container é
//            usado pelo StoreGate para armazenar/recuperar coleções de
//            descriptors (objetos const) no EDM.
// ============================================================================

#ifndef MuonChamberDetDescriptorCollection_h
#define MuonChamberDetDescriptorCollection_h

/** simulator includes **/
#include "MuonCell/MuonChamberDescriptorConverter.h"
#include "GaugiKernel/Collection.h"

namespace xAOD{
  // All objects inside of the data vector will be const
  typedef SG::Collection<xAOD::MuonChamberDescriptor> MuonChamberDetDescriptorCollection;
}
#endif
