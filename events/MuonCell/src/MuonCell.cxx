// ============================================================================
// Arquivo: src/MuonCell.cxx
// Descrição: Implementação dos métodos da classe xAOD::MuonChamberCell.
//            - Construtor parametrizado: inicializa posição/dimensões e zera
//              todas as variáveis dinâmicas (sinal, tempo, flags, etc.).
//            - reset(): reinicia todas as variáveis dinâmicas (mantém apenas
//              posição e dimensões).
//            - merge(): combina as informações de outra célula, somando
//              amplitudes/energias/sinais e fazendo médias ponderadas para
//              tempo, theta, phi, eta, deltaEta, deltaPhi, recoTime e
//              eficiência. As flags isMuon e isTrigger usam OR lógico.
// ============================================================================

#include "MuonCell/MuonChamberCell.h"

namespace xAOD {

//!=====================================================================
// Construtor: inicializa coordenadas e dimensões; zera o resto
//!=====================================================================
MuonChamberCell::MuonChamberCell(float x, float y, float z,
                                 float dx, float dy, float dz)
    : m_x(x), m_y(y), m_z(z),
      m_dx(dx), m_dy(dy), m_dz(dz),
      m_amplitude(0), m_time(0),
      m_isMuon(false),
      m_theta(0), m_phi(0),
      m_descriptor(nullptr),
      m_eta(0), m_deltaEta(0), m_deltaPhi(0),
      m_energy(0), m_trackLength(0), m_recoTime(0),
      m_signal(0), m_efficiency(0), m_isTrigger(false),
      m_layer(0), m_samplingName("")
{
}

//!=====================================================================
// Resetar informações dinâmicas do sinal
// (mantém posição, dimensões e ponteiro do descriptor)
//!=====================================================================
void MuonChamberCell::reset()
{
    m_amplitude = 0.0;
    m_time = 0.0;
    m_isMuon = false;
    m_theta = 0.0;
    m_phi = 0.0;

    m_eta = 0.0;
    m_deltaEta = 0.0;
    m_deltaPhi = 0.0;
    m_energy = 0.0;
    m_trackLength = 0.0;
    m_recoTime = 0.0;
    m_signal = 0.0;
    m_efficiency = 0.0;
    m_isTrigger = false;
    m_layer = 0;
    m_samplingName.clear();
}

//!=====================================================================
// Combinar informações de outra célula (merge)
//   - Amplitude/energia/sinal/trackLength: soma
//   - Tempo/recoTime/theta/phi/eta/deltaEta/deltaPhi/eficiência: média
//     (tempo é ponderado pela amplitude)
//   - isMuon, isTrigger: OR lógico
//   - layer: máximo
//   - samplingName: sobrescrito se o "other" tiver um
//!=====================================================================
void MuonChamberCell::merge(const MuonChamberCell& other)
{
    m_amplitude += other.m_amplitude;

    // Média ponderada pela amplitude (apenas se a soma > 0)
    if ((m_amplitude + other.m_amplitude) > 0) {
        m_time = (m_time * m_amplitude + other.m_time * other.m_amplitude) /
                 (m_amplitude + other.m_amplitude);
    }

    m_isMuon = m_isMuon || other.m_isMuon;
    m_theta = 0.5f * (m_theta + other.m_theta);
    m_phi   = 0.5f * (m_phi + other.m_phi);

    m_eta       = 0.5f * (m_eta + other.m_eta);
    m_deltaEta  = 0.5f * (m_deltaEta + other.m_deltaEta);
    m_deltaPhi  = 0.5f * (m_deltaPhi + other.m_deltaPhi);
    m_energy   += other.m_energy;
    m_trackLength += other.m_trackLength;
    m_recoTime  = 0.5f * (m_recoTime + other.m_recoTime);
    m_signal   += other.m_signal;
    m_efficiency = 0.5f * (m_efficiency + other.m_efficiency);
    m_isTrigger  = m_isTrigger || other.m_isTrigger;
    m_layer      = std::max(m_layer, other.m_layer);
    if (!other.m_samplingName.empty()) m_samplingName = other.m_samplingName;
}

} // namespace xAOD
