// ============================================================================
// Arquivo: MuonCell/MuonChamberDescriptorContainer.h
// Autora: Julya Pacheco Ferreira
// Descrição: Define o container padrão de descriptors de câmara de múon
//            (xAOD::MuonChamberDescriptorContainer), que herda de
//            SG::DataVector<xAOD::MuonChamberDescriptor>. Esse container é
//            usado pelo StoreGate para armazenar/recuperar coleções de
//            descriptors de câmara de múon.
// ============================================================================

#ifndef MUONCHAMBERDESCRIPTORCONTAINER_H
#define MUONCHAMBERDESCRIPTORCONTAINER_H

#include "MuonChamberDescriptorConverter.h"
#include "GaugiKernel/DataVector.h"

namespace xAOD {
    /// Container para MuonChamberDescriptor
    class MuonChamberDescriptorContainer : public SG::DataVector<MuonChamberDescriptor> {
      public:
        MuonChamberDescriptorContainer() = default;
        virtual ~MuonChamberDescriptorContainer() = default;
    };
}

#endif // MUONCHAMBERDESCRIPTORCONTAINER_H
