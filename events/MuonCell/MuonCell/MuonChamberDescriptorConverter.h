// ============================================================================
// Arquivo: MuonCell/MuonChamberDescriptorConverter.h
// Autora: Julya Pacheco Ferreira
// Descrição: Define a struct POD xAOD::MuonChamberDescriptor_t,
//            usada para persistir descriptors de câmara de múon em arquivos
//            ROOT, e a classe MuonChamberDescriptorConverter, responsável
//            por converter entre a representação de memória
//            (xAOD::MuonChamberDescriptor*) e a representação serializável
//            (MuonChamberDescriptor_t). A struct inclui:
//              - posição e dimensões (x,y,z,dx,dy,dz);
//              - identificação (hash, chamberType, sampling, layer, region,
//                side);
//              - coordenadas derivadas (η, φ, Δη, Δφ);
//              - configuração de BCIDs (bcid_start, bcid_end, bc_duration);
//              - parâmetros de operação (eficiência, ruído, resolução
//                temporal/espacial, velocidade de deriva, tensão, gás);
//              - energia/hits (energia estimada, energia de verdade,
//                hitCount, clusterSize, driftTime);
//              - forma de pulso (pulse).
// ============================================================================

#ifndef MUON_CHAMBER_DESCRIPTOR_CONVERTER_H
#define MUON_CHAMBER_DESCRIPTOR_CONVERTER_H

#include "MuonCell/MuonChamberDetDescriptor.h"
#include <vector>
#include <string>

namespace xAOD {

// --- Struct POD serializável (descriptor de câmara de múon) --------
struct MuonChamberDescriptor_t {
    // Posição e dimensões
    float x, y, z;
    float dx, dy, dz;

    // Identificação
    unsigned long hash;
    int chamberType;
    int sampling;
    int layer;
    int region;
    int side;

    // ---------- ADICIONAR ESTES CAMPOS! ----------
    // Coordenadas
    float eta;
    float phi;
    float deltaEta;
    float deltaPhi;
    // --------------------------------------------

    // BCID configuration
    int bcid_start;
    int bcid_end;
    float bc_duration;

    // Parâmetros de operação
    float efficiency;
    float noiseLevel;
    float timeResolution;
    float spatialResolution;
    float driftVelocity;
    float operatingVoltage;
    std::string gasMixture;

    // Energia e hits
    float energy;        // Energia estimada pelo filtro ótimo
    float truthEnergy;   // Energia verdadeira acumulada
    int hitCount;        // Número de hits
    int clusterSize;     // Tamanho do cluster
    float driftTime;     // Tempo de deriva

    // Forma de pulso
    std::vector<float> pulse;
};

// --- Conversor entre xAOD::MuonChamberDescriptor* e MuonChamberDescriptor_t
class MuonChamberDescriptorConverter {
public:
    // Serialização: MuonChamberDescriptor* → MuonChamberDescriptor_t
    bool convert(const MuonChamberDescriptor* desc,
                 MuonChamberDescriptor_t& desc_t);

    // Desserialização: MuonChamberDescriptor_t → MuonChamberDescriptor*
    bool convert(const MuonChamberDescriptor_t& desc_t,
                 MuonChamberDescriptor*& desc);
};

} // namespace xAOD

#endif
