// ============================================================================
// Arquivo: MuonConverter.h
// Autora: Julya Pacheco Ferreira
// Descrição: Define a struct POD (Plain Old Data) xAOD::Muon_t, usada para
//            persistir múons em arquivos ROOT (TTree) com campos primitivos,
//            e a classe MuonConverter, responsável por converter entre a
//            representação de memória (xAOD::Muon*) e a representação
//            serializável (Muon_t). A Muon_t inclui:
//              - variáveis cinemáticas (eta, phi, e, et, pt);
//              - carga e qualidade (charge, quality);
//              - isolamento e parâmetros de impacto (iso, d0, z0, σd0, σz0);
//              - flags de working point (isVeryLoose, isLoose, isMedium,
//                isTight);
//              - variáveis de qualidade do cluster (fracMax, chi2,
//                qOverPSignificance, rhoPrime);
//              - link para o cluster de origem (cluster_link).
// ============================================================================

#ifndef MuonConverter_h
#define MuonConverter_h

#include "Muon/Muon.h"
#include "MuonCluster/MuonClusterConverter.h"
#include <vector>

namespace xAOD {

// --- Struct POD serializável (usada para persistência em ROOT) -----
struct Muon_t {
    // Variáveis cinemáticas
    float eta{0};
    float phi{0};
    float e{0};
    float et{0};
    float pt{0};

    // Carga e qualidade
    int charge{0};
    int quality{0};

    // Isolamento e parâmetros de impacto
    float iso{0};
    float d0{0};
    float z0{0};
    float sigmaD0{0};
    float sigmaZ0{0};

    // Link para o cluster de origem
    int cluster_link{-1};

    // Flags de working point
    bool isVeryLoose{false};
    bool isLoose{false};
    bool isMedium{false};
    bool isTight{false};

    // Variáveis de qualidade do cluster
    float fracMax{0};
    float chi2{0};
    float qOverPSignificance{0};
    float rhoPrime{0};
};

// --- Conversor entre xAOD::Muon* e Muon_t --------------------------
class MuonConverter {
public:
    MuonConverter() = default;
    ~MuonConverter() = default;

    // Converte de xAOD::Muon* para Muon_t (serialização)
    bool convert(const Muon* from, Muon_t& to);

    // Converte de Muon_t para xAOD::Muon* (desserialização)
    bool convert(const Muon_t& from, Muon*& to);
};

} // namespace xAOD

#endif
