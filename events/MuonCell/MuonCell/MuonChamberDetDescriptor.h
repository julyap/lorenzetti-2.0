// ============================================================================
// Arquivo: MuonCell/MuonChamberDetDescriptor.h
// Descrição: Declaração da classe xAOD::MuonChamberDescriptor, que representa
//            a geometria e as propriedades de uma célula de câmara de múon no
//            EDM. Inclui:
//              - construtores (padrão, completo e de cópia);
//              - getters/setters para posição, dimensões, hash, chamberType,
//                sampling, layer, region, eficiência, ruído, resoluções,
//                gás, tensão, velocidade de deriva, BCIDs e duração de BC;
//              - acesso a contagens de hits, driftTime, clusterSize, edep e
//                tof por BCID;
//              - acesso ao pulso gerado (pulse(), setPulse(), tau());
//              - métodos utilitários (totalEdep, findMaxEnergyBCID,
//                loadShaperFile, generatePulse, addNoise);
//            A classe usa vetores paralelos por BCID para armazenar
//            hit_count, drift_time, cluster_size, time, edep e tof.
// ============================================================================

#ifndef MuonChamberDescriptor_h
#define MuonChamberDescriptor_h

#include "GaugiKernel/EDM.h"
#include "GaugiKernel/macros.h"
#include "TVector3.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <cmath>
#include <random>

namespace xAOD {

class MuonChamberDescriptor : public Gaugi::EDM
{
public:
    // Construtor padrão
    MuonChamberDescriptor();

    // Construtor completo (sem station)
    MuonChamberDescriptor(float x, float y, float z,
                          float dx, float dy, float dz,
                          unsigned long hash,
                          int chamberType,
                          int sampling,
                          int layer,
                          int region,
                          float bc_duration,
                          int bcid_start,
                          int bcid_end);

    // Construtor de cópia
    MuonChamberDescriptor(const MuonChamberDescriptor& other);

    // Método copy (cópia explícita, usado por containers)
    MuonChamberDescriptor* copy() const;

    // Destrutor virtual
    virtual ~MuonChamberDescriptor() = default;

    // --- Getters e setters de posição -----------------------------
    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }
    void setX(float v) { m_x = v; }
    void setY(float v) { m_y = v; }
    void setZ(float v) { m_z = v; }

    // --- Getters e setters de dimensões ---------------------------
    float dx() const { return m_dx; }
    float dy() const { return m_dy; }
    float dz() const { return m_dz; }
    void setDx(float v) { m_dx = v; }
    void setDy(float v) { m_dy = v; }
    void setDz(float v) { m_dz = v; }

    // --- Hash e tipo de câmara ------------------------------------
    unsigned long hash() const { return m_hash; }
    void setHash(unsigned long h) { m_hash = h; }

    int chamberType() const { return m_chamberType; }
    void setChamberType(int v) { m_chamberType = v; }

    // APENAS SAMPLING (sem station)
    int sampling() const { return m_sampling; }
    void setSampling(int v) { m_sampling = v; }

    int layer() const { return m_layer; }
    void setLayer(int v) { m_layer = v; }

    int region() const { return m_region; }
    void setRegion(int v) { m_region = v; }

    // --- Propriedades físicas -------------------------------------
    float efficiency() const { return m_efficiency; }
    void setEfficiency(float v) { m_efficiency = v; }

    float noiseLevel() const { return m_noiseLevel; }
    void setNoiseLevel(float v) { m_noiseLevel = v; }

    float timeResolution() const { return m_timeResolution; }
    void setTimeResolution(float v) { m_timeResolution = v; }

    float spatialResolution() const { return m_spatialResolution; }
    void setSpatialResolution(float v) { m_spatialResolution = v; }

    std::string gasMixture() const { return m_gasMixture; }
    void setGasMixture(const std::string &v) { m_gasMixture = v; }

    float operatingVoltage() const { return m_operatingVoltage; }
    void setOperatingVoltage(float v) { m_operatingVoltage = v; }

    float driftVelocity() const { return m_driftVelocity; }
    void setDriftVelocity(float v) { m_driftVelocity = v; }

    // --- BCIDs ----------------------------------------------------
    int bcid_start() const { return m_bcid_start; }
    int bcid_end() const { return m_bcid_end; }
    float bc_duration() const { return m_bc_duration; }

    // Número total de BCIDs
    int n_bcids() const { return m_bcid_end - m_bcid_start + 1; }

    // --- Hit info por BCID ----------------------------------------
    int hitCount(int bcid) const;
    void setHitCount(int bcid, int count);

    float driftTime(int bcid) const;
    void setDriftTime(int bcid, float t);

    int clusterSize(int bcid) const;
    void setClusterSize(int bcid, int s);

    // --- Energia e tempo por BCID ---------------------------------
    float edep(int bcid) const;
    void edep(int bcid, float value);

    float tof(int bcid) const;
    void tof(int bcid, float value);

    // --- Pulso ----------------------------------------------------
    float e() const { return m_e; }
    void setE(float value) { m_e = value; }

    const std::vector<float>& pulse() const { return m_pulse; }
    void setPulse(const std::vector<float>& pulse) { m_pulse = pulse; }

    float tau() const { return m_tau; }
    void setTau(float value) { m_tau = value; }

    // --- Tempo por BCID -------------------------------------------
    float time(int bcid) const;

    // --- Shaper ---------------------------------------------------
    bool loadShaperFile(const std::string& filename);
    void generatePulse();
    bool hasPulse() const { return !m_pulse.empty(); }
    float totalEdep() const;
    int findMaxEnergyBCID() const;
    void addNoise(float noiseLevel);

private:
    unsigned long m_hash;

    // Chamber geometry
    float m_x, m_y, m_z;
    float m_dx, m_dy, m_dz;

    int m_chamberType;
    int m_sampling;      // Sampling ID (1-7) - substitui station
    int m_layer;
    int m_region;

    // Chamber properties
    float m_efficiency;
    float m_noiseLevel;
    float m_timeResolution;
    float m_spatialResolution;

    // Muon-specific
    std::string m_gasMixture;
    float m_operatingVoltage;
    float m_driftVelocity;

    // Energy and time information
    float m_e;
    std::vector<float> m_pulse;
    float m_tau;

    // Bunch crossing info
    int m_bcid_start, m_bcid_end;
    float m_bc_duration;

    // Hit/cluster info (vetores paralelos por BCID)
    std::vector<int> m_hit_count;
    std::vector<float> m_drift_time;
    std::vector<int> m_cluster_size;
    std::vector<float> m_time;

    // Energy deposit and time of flight per BCID
    std::vector<float> m_edep;
    std::vector<float> m_tof;

    // Shaper file
    std::string m_shaperFile;
    std::vector<float> m_pulseTemplate;
};

// ============================================================================
// Implementações inline
// ============================================================================

// --- Soma de toda a energia depositada ------------------------------
inline float MuonChamberDescriptor::totalEdep() const
{
    float total = 0.0f;
    for (const auto& e : m_edep) {
        total += e;
    }
    return total;
}

// --- BCID com maior energia depositada ------------------------------
inline int MuonChamberDescriptor::findMaxEnergyBCID() const
{
    int max_bcid = m_bcid_start;
    float max_edep = 0.0f;

    for (int bcid = m_bcid_start; bcid <= m_bcid_end; ++bcid) {
        int idx = bcid - m_bcid_start;
        if (idx >= 0 && idx < (int)m_edep.size()) {
            if (m_edep[idx] > max_edep) {
                max_edep = m_edep[idx];
                max_bcid = bcid;
            }
        }
    }
    return max_bcid;
}

// --- Carrega o template de pulso de um arquivo ----------------------
inline bool MuonChamberDescriptor::loadShaperFile(const std::string& filename)
{
    m_shaperFile = filename;
    m_pulseTemplate.clear();

    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    float value;
    while (file >> value) {
        m_pulseTemplate.push_back(value);
    }
    file.close();

    return !m_pulseTemplate.empty();
}

// --- Gera o pulso a partir do template e da energia por BCID -------
inline void MuonChamberDescriptor::generatePulse()
{
    m_pulse.clear();

    int nbunchs = n_bcids();
    if (nbunchs <= 0) return;

    m_pulse.resize(nbunchs, 0.0f);

    float total_energy = totalEdep();
    if (total_energy <= 0) return;

    int max_bcid = findMaxEnergyBCID();

    if (!m_pulseTemplate.empty()) {
        int template_size = (int)m_pulseTemplate.size();

        // Convolução do template com a energia por BCID
        for (int bcid = m_bcid_start; bcid <= m_bcid_end; ++bcid) {
            int idx = bcid - m_bcid_start;
            float sum = 0.0f;

            for (int t = 0; t < template_size; ++t) {
                int hist_bcid = bcid - (t - template_size/2);
                if (hist_bcid >= m_bcid_start && hist_bcid <= m_bcid_end) {
                    int hist_idx = hist_bcid - m_bcid_start;
                    sum += m_edep[hist_idx] * m_pulseTemplate[t];
                }
            }
            m_pulse[idx] = sum;
        }
    } else {
        // Fallback: pulso gaussiano centrado em max_bcid
        for (int bcid = m_bcid_start; bcid <= m_bcid_end; ++bcid) {
            int idx = bcid - m_bcid_start;
            int dist = std::abs(bcid - max_bcid);

            if (dist < 5) {
                m_pulse[idx] = total_energy * std::exp(-dist * dist / 2.0f);
            }
        }
    }

    // Normaliza o pico para a energia total
    float max_pulse = 0.0f;
    for (const auto& p : m_pulse) {
        if (p > max_pulse) max_pulse = p;
    }

    if (max_pulse > 0) {
        for (auto& p : m_pulse) {
            p = p * total_energy / max_pulse;
        }
    }
}

// --- Adiciona ruído uniforme ao pulso -------------------------------
inline void MuonChamberDescriptor::addNoise(float noiseLevel)
{
    if (m_pulse.empty()) return;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    for (auto& p : m_pulse) {
        float noise = dis(gen) * noiseLevel;
        p += noise;
        if (p < 0) p = 0;
    }
}

} // namespace xAOD

#endif
