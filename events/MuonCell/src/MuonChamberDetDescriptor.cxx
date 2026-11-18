// ============================================================================
// Arquivo: src/MuonChamberDetDescriptor.cxx
// Autora: Julya Pacheco Ferreira
// Descrição: Implementação da classe xAOD::MuonChamberDescriptor, que
//            representa o "descriptor" geométrico/físico de uma célula de
//            múon. Armazena posição/dimensões, identificação (hash,
//            chamberType, sampling, layer, region), parâmetros de operação
//            (eficiência, ruído, resoluções, gás, tensão, velocidade de
//            deriva), energia estimada (e) e tempo (tau), além de vetores
//            indexados por BCID com hitCount, driftTime, clusterSize, time,
//            edep e tof, e a forma de pulso (pulse).
//            Fornece:
//              - construtores (default, completo e cópia);
//              - copy() para clonagem;
//              - getters/setters por BCID, com validação de índice
//                (validateIndex).
// ============================================================================

#include "MuonCell/MuonChamberDetDescriptor.h"
#include <stdexcept>
#include <iostream>

namespace xAOD {

//!=====================================================================
// Utilitário: valida o índice BCID e retorna o índice correspondente no
// vetor interno (index = bcid - bcid_start). Lança std::out_of_range se
// fora dos limites.
//!=====================================================================
inline int validateIndex(int bcid, int bcid_start, size_t vector_size) {
    int index = bcid - bcid_start;
    if (index < 0 || index >= static_cast<int>(vector_size)) {
        std::string error = "BCID fora do range. BCID=" +
                           std::to_string(bcid) +
                           ", start=" + std::to_string(bcid_start) +
                           ", índice=" + std::to_string(index) +
                           ", tamanho=" + std::to_string(vector_size);
        throw std::out_of_range(error);
    }
    return index;
}

//!=====================================================================
// Construtor padrão: inicializa tudo em zero/valores neutros e
// dimensiona os vetores internos com 1 BCID (evita divisão por zero).
//!=====================================================================
MuonChamberDescriptor::MuonChamberDescriptor() :
    Gaugi::EDM(),
    m_hash(0),
    m_x(0), m_y(0), m_z(0),
    m_dx(0), m_dy(0), m_dz(0),
    m_chamberType(0),
    m_sampling(0),
    m_layer(0),
    m_region(0),
    m_efficiency(1.0f),
    m_noiseLevel(0.0f),
    m_timeResolution(0.0f),
    m_spatialResolution(0.0f),
    m_gasMixture(""),
    m_operatingVoltage(0.0f),
    m_driftVelocity(0.0f),
    m_e(0.0f),
    m_tau(0.0f),
    m_bc_duration(25.0f),
    m_bcid_start(0),
    m_bcid_end(0)
{
    int total_bc = m_bcid_end - m_bcid_start + 1;
    if (total_bc <= 0) {
        total_bc = 1;  // Evitar divisão por zero
        m_bcid_end = m_bcid_start;
    }

    // Redimensiona todos os vetores internos
    m_hit_count.resize(total_bc, 0);
    m_drift_time.resize(total_bc, 0.0f);
    m_cluster_size.resize(total_bc, 0);
    m_time.resize(total_bc, 0.0f);
    m_edep.resize(total_bc, 0.0f);
    m_tof.resize(total_bc, 0.0f);
}

//!=====================================================================
// Construtor completo (sem station): inicializa posição/dimensões,
// identificação, parâmetros de operação e dimensiona os vetores internos
// com base no range de BCIDs. Lança exceção se bcid_start > bcid_end ou
// se o intervalo for inválido. Também pré-calcula o vetor de tempos
// (m_time) como (bcid - 0.5) * bc_duration.
//!=====================================================================
MuonChamberDescriptor::MuonChamberDescriptor(float x, float y, float z,
                                           float dx, float dy, float dz,
                                           unsigned long hash,
                                           int chamberType,
                                           int sampling,
                                           int layer,
                                           int region,
                                           float bc_duration,
                                           int bcid_start,
                                           int bcid_end) :
    Gaugi::EDM(),
    m_hash(hash),
    m_x(x), m_y(y), m_z(z),
    m_dx(dx), m_dy(dy), m_dz(dz),
    m_chamberType(chamberType),
    m_sampling(sampling),
    m_layer(layer),
    m_region(region),
    m_efficiency(1.0f),
    m_noiseLevel(0.0f),
    m_timeResolution(0.0f),
    m_spatialResolution(0.0f),
    m_gasMixture(""),
    m_operatingVoltage(0.0f),
    m_driftVelocity(0.0f),
    m_e(0.0f),
    m_tau(0.0f),
    m_bc_duration(bc_duration),
    m_bcid_start(bcid_start),
    m_bcid_end(bcid_end)
{
    if (bcid_start > bcid_end) {
        throw std::runtime_error("bcid_start > bcid_end no construtor");
    }

    int total_bc = m_bcid_end - m_bcid_start + 1;
    if (total_bc <= 0) {
        throw std::runtime_error("Intervalo BCID inválido: total_bc <= 0");
    }

    // Redimensiona todos os vetores internos
    m_hit_count.resize(total_bc, 0);
    m_drift_time.resize(total_bc, 0.0f);
    m_cluster_size.resize(total_bc, 0);
    m_time.resize(total_bc, 0.0f);
    m_edep.resize(total_bc, 0.0f);
    m_tof.resize(total_bc, 0.0f);

    // Pré-calcula o vetor de tempos (tempo no centro de cada BCID)
    float start = (m_bcid_start - 0.5f) * m_bc_duration;
    float step = m_bc_duration;
    for (int t = 0; t < total_bc; ++t) {
        m_time[t] = start + step * t;
    }
}

//!=====================================================================
// Construtor de cópia: copia todos os membros (incluindo os vetores
// internos e o pulse).
//!=====================================================================
MuonChamberDescriptor::MuonChamberDescriptor(const MuonChamberDescriptor& other) :
    Gaugi::EDM(other),
    m_hash(other.m_hash),
    m_x(other.m_x), m_y(other.m_y), m_z(other.m_z),
    m_dx(other.m_dx), m_dy(other.m_dy), m_dz(other.m_dz),
    m_chamberType(other.m_chamberType),
    m_sampling(other.m_sampling),
    m_layer(other.m_layer),
    m_region(other.m_region),
    m_efficiency(other.m_efficiency),
    m_noiseLevel(other.m_noiseLevel),
    m_timeResolution(other.m_timeResolution),
    m_spatialResolution(other.m_spatialResolution),
    m_gasMixture(other.m_gasMixture),
    m_operatingVoltage(other.m_operatingVoltage),
    m_driftVelocity(other.m_driftVelocity),
    m_e(other.m_e),
    m_tau(other.m_tau),
    m_bc_duration(other.m_bc_duration),
    m_bcid_start(other.m_bcid_start),
    m_bcid_end(other.m_bcid_end),
    m_hit_count(other.m_hit_count),
    m_drift_time(other.m_drift_time),
    m_cluster_size(other.m_cluster_size),
    m_time(other.m_time),
    m_edep(other.m_edep),
    m_tof(other.m_tof),
    m_pulse(other.m_pulse)
{
}

//!=====================================================================
// copy(): clona o descriptor usando o construtor de cópia.
//!=====================================================================
MuonChamberDescriptor* MuonChamberDescriptor::copy() const
{
    return new MuonChamberDescriptor(*this);
}

//!=====================================================================
// Getters/Setters por BCID (todos com validateIndex)
//!=====================================================================

// --- hitCount -----------------------------------------------------
int MuonChamberDescriptor::hitCount(int bcid) const
{
    int index = validateIndex(bcid, m_bcid_start, m_hit_count.size());
    return m_hit_count[index];
}

void MuonChamberDescriptor::setHitCount(int bcid, int count)
{
    int index = validateIndex(bcid, m_bcid_start, m_hit_count.size());
    m_hit_count[index] = count;
}

// --- driftTime ----------------------------------------------------
float MuonChamberDescriptor::driftTime(int bcid) const
{
    int index = validateIndex(bcid, m_bcid_start, m_drift_time.size());
    return m_drift_time[index];
}

void MuonChamberDescriptor::setDriftTime(int bcid, float t)
{
    int index = validateIndex(bcid, m_bcid_start, m_drift_time.size());
    m_drift_time[index] = t;
}

// --- clusterSize --------------------------------------------------
int MuonChamberDescriptor::clusterSize(int bcid) const
{
    int index = validateIndex(bcid, m_bcid_start, m_cluster_size.size());
    return m_cluster_size[index];
}

void MuonChamberDescriptor::setClusterSize(int bcid, int s)
{
    int index = validateIndex(bcid, m_bcid_start, m_cluster_size.size());
    m_cluster_size[index] = s;
}

// --- edep ---------------------------------------------------------
float MuonChamberDescriptor::edep(int bcid) const
{
    int index = validateIndex(bcid, m_bcid_start, m_edep.size());
    return m_edep[index];
}

void MuonChamberDescriptor::edep(int bcid, float value)
{
    int index = validateIndex(bcid, m_bcid_start, m_edep.size());
    m_edep[index] = value;
}

// --- tof ----------------------------------------------------------
float MuonChamberDescriptor::tof(int bcid) const
{
    int index = validateIndex(bcid, m_bcid_start, m_tof.size());
    return m_tof[index];
}

void MuonChamberDescriptor::tof(int bcid, float value)
{
    int index = validateIndex(bcid, m_bcid_start, m_tof.size());
    m_tof[index] = value;
}

// --- time (somente leitura) ---------------------------------------
float MuonChamberDescriptor::time(int bcid) const
{
    int index = validateIndex(bcid, m_bcid_start, m_time.size());
    return m_time[index];
}

} // namespace xAOD
