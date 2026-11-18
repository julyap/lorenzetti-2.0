// ============================================================================
// Arquivo: MuonCell/MuonEnumeration.h
// Autora: Julya Pacheco Ferreira
// Descrição: Define os enums de enumeração usados pelo subsistema de câmara
//            de múon:
//              - MuonChamberType: MDT, RPC, TGC;
//              - MuonStation: BI, BM, BO, EI, EM, EO (barrel/endcap, inner/
//                middle/outer);
//              - MuonLayer: Inner, Middle, Outer;
//              - MuonRegion: Barrel (0), EndcapA (1), EndcapC (2).
//            Os enums usam uint8_t como tipo base (economia de memória) e
//            são usados na identificação de células e descriptors.
// ============================================================================

#ifndef MUONENUMERATION_H
#define MUONENUMERATION_H

#include <cstdint>  // Para uint8_t

namespace xAOD {

  // Tipos de câmara de múons
  enum class MuonChamberType : uint8_t {
      MDT,   // Monitored Drift Tubes
      RPC,   // Resistive Plate Chambers
      TGC,   // Thin Gap Chambers
  };

  // Estações de múons
  enum class MuonStation : uint8_t {
      BI,   // Barrel Inner
      BM,   // Barrel Middle
      BO,   // Barrel Outer
      EI,   // Endcap Inner
      EM,   // Endcap Middle
      EO,   // Endcap Outer

  };

  // Camadas
  enum class MuonLayer : uint8_t {
      InnerLayer,
      MiddleLayer,
      OuterLayer
  };

  // Regiões
  enum class MuonRegion : uint8_t {
      Barrel = 0,      // Região Central
      EndcapA = 1,     // Endcap positivo (A)
      EndcapC = 2      // Endcap negativo (C)
  };

} // namespace xAOD

#endif
