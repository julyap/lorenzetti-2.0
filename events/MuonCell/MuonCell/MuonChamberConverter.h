// ============================================================================
// Arquivo: MuonCell/MuonChamberConverter.h
// Descrição: Define a struct POD xAOD::MuonChamber_t, usada
//            para persistir células de câmara de múon em arquivos ROOT, e a
//            classe MuonChamberConverter, responsável por converter entre a
//            representação de memória (xAOD::MuonChamberCell*) e a
//            representação serializável (MuonChamber_t). A struct inclui:
//              - posição e dimensões (x,y,z,dx,dy,dz);
//              - propriedades do sinal (amplitude, tempo);
//              - hash e link para o descriptor;
//              - identificação (chamberType, region, sampling, detector,
//                layer);
//              - campos adicionais do MuonChamberCell (isMuon, θ, φ, η, Δη,
//                Δφ, energia, comprimento da track, tempo de reco, sinal,
//                eficiência, flag de trigger).
// ============================================================================

#ifndef MUON_CHAMBER_CONVERTER_H
#define MUON_CHAMBER_CONVERTER_H

#include "MuonCell/MuonChamberCell.h"
#include "MuonCell/MuonChamberDescriptorConverter.h"
#include <cstring>

namespace xAOD {

// --- Struct POD serializável (célula de câmara de múon) ------------
struct MuonChamber_t {
    // Posição e dimensões
    float x, y, z;
    float dx, dy, dz;

    // Propriedades do sinal
    float amplitude;
    float time;

    // Links e identificadores
    unsigned long hash;
    unsigned long descriptor_link;

    // Campos de identificação da câmara
    int chamberType;
    int region;

    // ===== CAMPOS PRINCIPAIS =====
    int sampling;       // ID da amostragem (1-7)
    int detector;       // Tipo de detector (0=MDT,1=RPC,2=TGC)
    int layer;          // Camada

    // Campos adicionais do MuonChamberCell
    bool isMuon;
    float theta;
    float phi;
    float eta;
    float deltaEta;
    float deltaPhi;
    float energy;
    float trackLength;
    float recoTime;
    float signal;
    float efficiency;
    bool isTrigger;


    // Construtor padrão (inicializa todos os campos)
    MuonChamber_t()
        : x(0), y(0), z(0), dx(0), dy(0), dz(0),
          amplitude(0), time(0), hash(0), descriptor_link(0),
          chamberType(0), region(0), sampling(0), detector(0),
          layer(0),
          isMuon(false), theta(0), phi(0), eta(0), deltaEta(0),
          deltaPhi(0), energy(0), trackLength(0), recoTime(0),
          signal(0), efficiency(0), isTrigger(false) {}
};

// --- Conversor entre xAOD::MuonChamberCell* e MuonChamber_t --------
class MuonChamberConverter {
public:
    // Serialização: MuonChamberCell* → MuonChamber_t
    bool convert( const MuonChamberCell *cell, MuonChamber_t &cell_t);

    // Desserialização: MuonChamber_t → MuonChamberCell*
    bool convert( const MuonChamber_t &cell_t, MuonChamberCell *&cell );

    // Desserialização a partir de um descriptor serializado
    bool convert( const MuonChamberDescriptor_t &desc_t, MuonChamberCell *&cell );

private:
    // Cálculos auxiliares de η e φ a partir de (x,y,z)
    float calcEta(float x, float y, float z);
    float calcPhi(float x, float y);
};

} // namespace xAOD

#endif
