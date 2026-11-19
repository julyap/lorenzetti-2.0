// ============================================================================
// Arquivo: MuonDefs/MuonDefs.h
// Descrição: Definições auxiliares para o subsistema de múons no EDM.
//            Este header fornece:
//              - A enumeração MuonSampling (Inner/Middle/Outer), que
//                complementa as enumerações já existentes (MuonChamberType,
//                MuonRegion, MuonLayer, MuonStation).
//              - Funções utilitárias de conversão enum → string
//                (MuonChamberTypeToString, MuonRegionToString,
//                MuonSamplingToString, MuonLayerToString,
//                MuonStationToString).
//              - Funções de conversão entre enumerações
//                (StationToSampling, SamplingToLayer).
//              - A struct MuonChamberId, que agrega identificadores
//                (tipo de câmara, sampling, layer, região, índices
//                eta/phi e multiLayer) e fornece um método toString().
//            Essas definições são usadas para nomear, indexar e comparar
//            câmaras de múon em toda a cadeia de simulação/reconstrução.
// ============================================================================

#ifndef MUONDEFS_H
#define MUONDEFS_H

#include "MuonCell/MuonEnumeration.h"
#include <cstdint>
#include <string>

namespace xAOD {

  // ============================================
  // NOVO ENUM: MuonSampling (Inner/Middle/Outer)
  // ============================================
  // Enumeração simplificada de sampling (camada interna, média e externa),
  // usada em conjunto com MuonStation para identificação de câmaras.
  enum class MuonSampling : uint8_t {
    Inner = 1,   // Camada interna
    Middle = 2,  // Camada média
    Outer = 3    // Camada externa
  };

  // ============================================
  // FUNÇÕES AUXILIARES
  // ============================================

  // Converte MuonChamberType para string (usando enum existente)
  inline const char* MuonChamberTypeToString(MuonChamberType type) {
    switch(type) {
      case MuonChamberType::MDT:   return "MDT";
      case MuonChamberType::RPC:   return "RPC";
      case MuonChamberType::TGC:   return "TGC";
      default:                     return "UNKNOWN";
    }
  }

  // Converte MuonRegion para string (usando enum existente)
  inline const char* MuonRegionToString(MuonRegion region) {
    switch(region) {
      case MuonRegion::Barrel:   return "Barrel";
      case MuonRegion::EndcapA:  return "EndcapA";
      case MuonRegion::EndcapC:  return "EndcapC";
      default:                   return "UNKNOWN";
    }
  }

  // Converte MuonSampling para string (NOVO)
  inline const char* MuonSamplingToString(MuonSampling sampling) {
    switch(sampling) {
      case MuonSampling::Inner:   return "Inner";
      case MuonSampling::Middle:  return "Middle";
      case MuonSampling::Outer:   return "Outer";
      default:                    return "UNKNOWN";
    }
  }

  // Converte MuonLayer para string (usando enum existente)
  inline const char* MuonLayerToString(MuonLayer layer) {
    switch(layer) {
      case MuonLayer::InnerLayer:   return "Inner";
      case MuonLayer::MiddleLayer:  return "Middle";
      case MuonLayer::OuterLayer:   return "Outer";
      default:                      return "UNKNOWN";
    }
  }

  // Converte MuonStation para string (usando enum existente)
  inline const char* MuonStationToString(MuonStation station) {
    switch(station) {
      case MuonStation::BI:   return "BI";
      case MuonStation::BM:   return "BM";
      case MuonStation::BO:   return "BO";
      case MuonStation::EI:   return "EI";
      case MuonStation::EM:   return "EM";
      case MuonStation::EO:   return "EO";
      default:                return "UNKNOWN";
    }
  }

  // ============================================
  // FUNÇÕES DE CONVERSÃO
  // ============================================

  // Converte MuonStation para MuonSampling
  // (BI/EI → Inner, BM/EM → Middle, BO/EO → Outer)
  inline MuonSampling StationToSampling(MuonStation station) {
    switch(station) {
      case MuonStation::BI:
      case MuonStation::EI:
        return MuonSampling::Inner;
      case MuonStation::BM:
      case MuonStation::EM:
        return MuonSampling::Middle;
      case MuonStation::BO:
      case MuonStation::EO:
        return MuonSampling::Outer;
      default:
        return MuonSampling::Inner;
    }
  }

  // Converte MuonSampling para MuonLayer
  // (Inner → InnerLayer, Middle → MiddleLayer, Outer → OuterLayer)
  inline MuonLayer SamplingToLayer(MuonSampling sampling) {
    switch(sampling) {
      case MuonSampling::Inner:  return MuonLayer::InnerLayer;
      case MuonSampling::Middle: return MuonLayer::MiddleLayer;
      case MuonSampling::Outer:  return MuonLayer::OuterLayer;
      default:                   return MuonLayer::InnerLayer;
    }
  }

  // ============================================
  // MUON CHAMBER ID STRUCTURE
  // ============================================

  // Estrutura para identificar uma câmara de múons
  struct MuonChamberId {
    MuonChamberType chamberType;  // Tipo de câmara (MDT/RPC/TGC/...)
    MuonSampling sampling;        // NOVO: usando MuonSampling (Inner/Middle/Outer)
    MuonLayer layer;              // Camada (InnerLayer/MiddleLayer/OuterLayer)
    MuonRegion region;            // Região (Barrel/EndcapA/EndcapC)
    uint8_t etaIndex;             // Índice no eixo eta
    uint8_t phiIndex;             // Índice no eixo phi
    uint8_t multiLayer;           // Identificador de multi-camada

    // Construtor padrão: inicializa com valores neutros
    MuonChamberId() :
      chamberType(MuonChamberType::MDT),
      sampling(MuonSampling::Inner),
      layer(MuonLayer::InnerLayer),
      region(MuonRegion::Barrel),
      etaIndex(0),
      phiIndex(0),
      multiLayer(0) {}

    // Construtor completo (usando MuonSampling diretamente)
    MuonChamberId(MuonChamberType ct, MuonSampling samp, MuonLayer ly,
                  MuonRegion reg, uint8_t eta=0, uint8_t phi=0, uint8_t ml=0) :
      chamberType(ct), sampling(samp), layer(ly), region(reg),
      etaIndex(eta), phiIndex(phi), multiLayer(ml) {}

    // Construtor alternativo usando MuonStation (para compatibilidade)
    // Converte automaticamente a estação para MuonSampling via StationToSampling
    MuonChamberId(MuonChamberType ct, MuonStation st, MuonLayer ly,
                  MuonRegion reg, uint8_t eta=0, uint8_t phi=0, uint8_t ml=0) :
      chamberType(ct),
      sampling(StationToSampling(st)),
      layer(ly),
      region(reg),
      etaIndex(eta),
      phiIndex(phi),
      multiLayer(ml) {}

    // Gera uma string identificadora: "<ChamberType>_<Sampling>_<Layer>_<Region>"
    std::string toString() const {
      std::string result = std::string(MuonChamberTypeToString(chamberType)) + "_";
      result += std::string(MuonSamplingToString(sampling)) + "_";
      result += std::string(MuonLayerToString(layer)) + "_";
      result += std::string(MuonRegionToString(region));
      return result;
    }
  };

} // namespace xAOD

#endif // MUONDEFS_H
