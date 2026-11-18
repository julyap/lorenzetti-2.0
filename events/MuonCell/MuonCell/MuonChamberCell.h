// ============================================================================
// Arquivo: MuonCell/MuonChamberCell.h
// Autora: Julya Pacheco Ferreira
// Descrição: Declaração da classe xAOD::MuonChamberCell, que representa uma
//            célula de câmara de múon no EDM. Herda de Gaugi::EDM e armazena
//            posição (x,y,z), dimensões (dx,dy,dz), propriedades do sinal
//            (amplitude, tempo), identificação de múon (isMuon), direção da
//            track (θ, φ) e referência ao descriptor de origem. Também
//            inclui campos auxiliares usados no merge de células (η, Δη, Δφ,
//            energia, comprimento da track, tempo reconstruído, sinal,
//            eficiência, flag de trigger, layer e nome da sampling).
// ============================================================================

#ifndef MUONCHAMBERCELL_H
#define MUONCHAMBERCELL_H

/** Simulator includes **/
#include "MuonCell/MuonEnumeration.h"
#include "MuonCell/MuonChamberDescriptorContainer.h"
#include "GaugiKernel/EDM.h"
#include "GaugiKernel/macros.h"
#include "TVector3.h"
#include <string>

namespace xAOD {

class MuonChamberDescriptor; // Forward declaration

class MuonChamberCell : public Gaugi::EDM
{
public:
    /** Constructors **/
    MuonChamberCell() = default;
    MuonChamberCell(float x, float y, float z, float dx, float dy, float dz);

    /** Destructor **/
    ~MuonChamberCell() = default;

    /** Position getters/setters **/
    PRIMITIVE_SETTER_AND_GETTER(float, m_x, setX, x);
    PRIMITIVE_SETTER_AND_GETTER(float, m_y, setY, y);
    PRIMITIVE_SETTER_AND_GETTER(float, m_z, setZ, z);

    /** Cell size getters/setters **/
    PRIMITIVE_SETTER_AND_GETTER(float, m_dx, setDx, dx);
    PRIMITIVE_SETTER_AND_GETTER(float, m_dy, setDy, dy);
    PRIMITIVE_SETTER_AND_GETTER(float, m_dz, setDz, dz);

    /** Signal properties **/
    PRIMITIVE_SETTER_AND_GETTER(float, m_amplitude, setAmplitude, amplitude);
    PRIMITIVE_SETTER_AND_GETTER(float, m_time, setTime, time);

    /** Muon identification **/
    PRIMITIVE_SETTER_AND_GETTER(bool, m_isMuon, setIsMuon, isMuon);

    /** Track direction **/
    PRIMITIVE_SETTER_AND_GETTER(float, m_theta, setTheta, theta);
    PRIMITIVE_SETTER_AND_GETTER(float, m_phi, setPhi, phi);

    /** Descriptor access **/
    // --- Referência ao descriptor de origem (geometria) -----------
    const MuonChamberDescriptor* descriptor() const { return m_descriptor; }
    void setDescriptor(const MuonChamberDescriptor* descriptor) { m_descriptor = descriptor; }

    /** Compute global position **/
    // --- Posição global como TVector3 -----------------------------
    TVector3 globalPosition() const { return TVector3(m_x, m_y, m_z); }

    /** Methods for MuonCellMerge **/
    // --- Reset e merge (usados na combinação de células) ---------
    void reset();
    void merge(const MuonChamberCell& other);

    /** Extra fields required for merging **/
    float m_eta{0};          // pseudorapidez da célula
    float m_deltaEta{0};     // largura em η
    float m_deltaPhi{0};     // largura em φ
    float m_energy{0};       // energia depositada
    float m_trackLength{0};  // comprimento da track na célula
    float m_recoTime{0};     // tempo reconstruído
    float m_signal{0};       // sinal (amplitude calibrada)
    float m_efficiency{0};   // eficiência de detecção
    bool  m_isTrigger{false};// flag de trigger
    int   m_layer{0};        // camada
    std::string m_samplingName;  // nome da sampling (ex.: MDT_Inner_Barrel)

private:
    /** Position coordinates **/
    float m_x{0}, m_y{0}, m_z{0};

    /** Cell dimensions **/
    float m_dx{0}, m_dy{0}, m_dz{0};

    /** Signal properties **/
    float m_amplitude{0};
    float m_time{0};

    /** Muon identification **/
    bool m_isMuon{false};

    /** Track direction **/
    float m_theta{0};
    float m_phi{0};

    /** Associated raw information **/
    // --- Descriptor que originou a célula (geometria, BCIDs etc.) --
    const MuonChamberDescriptor* m_descriptor{nullptr};
};

} // namespace xAOD

#endif // MUONCHAMBERCELL_H
