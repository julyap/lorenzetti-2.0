// ============================================================================
// Arquivo: MuonHit/src/MuonHit.cxx
// Descrição: Implementação da classe xAOD::MuonHit.
//
//            THRESHOLDS POR DETECTOR (baseados na física de cada câmara):
//              - MDT (Ar/CO2, 3 bar): deposição ~10–15 keV  -> threshold  5 keV
//              - RPC (gap 2 mm)     : deposição ~0.5–1 keV  -> threshold 0.2 keV
//              - TGC (gap 2.8 mm)   : deposição ~1–2 keV    -> threshold 0.5 keV
//
//            Steps abaixo do threshold do detector são DESCARTADOS.
//
//            DIAGNÓSTICO DE TEMPO:
//              [MUON TIME DEBUG] imprime os primeiros 50 steps com
//              detector, região, tempo, janela de BCID e resultado de
//              find(t). Serve para confirmar se RPC/TGC caem fora da
//              janela temporal e são descartados por find(t) == -1.
//
//            BUG CORRIGIDO no find():
//              Antes: if (index >= (int)m_time.size() - 1) return -1;
//              Agora: if (index >= (int)m_time.size())     return -1;
//              (a última borda da janela agora é aceita)
// ============================================================================

#include "MuonHit/MuonHit.h"

#include "MuonDefs.h"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <tuple>

using namespace xAOD;


// ============================================================
// HELPERS DE THRESHOLD
// ============================================================

//! Retorna o threshold de energia (em MeV) para um dado detector.
//! Valores baseados na deposição típica de múon por camada:
//!   MDT: ~10–15 keV  -> threshold  5 keV
//!   RPC: ~0.5–1 keV  -> threshold 0.2 keV
//!   TGC: ~1–2 keV    -> threshold 0.5 keV
static float thresholdForDetector(MuonChamberType det)
{
    switch (det) {
        case MuonChamberType::MDT: return 0.005f;  //  5 keV
        case MuonChamberType::RPC: return 0.0002f; // 0.2 keV
        case MuonChamberType::TGC: return 0.0005f; // 0.5 keV
        default:                   return 0.001f;  //  1 keV (fallback)
    }
}

//! Threshold de debug (apenas para imprimir FILL/Z NEGATIVO).
//! 1 keV: qualquer step físico real de múon passa; ruído e-23 MeV não.
static constexpr float kDebugThresholdMeV = 0.001f;


// ============================================================
// CONSTRUTOR PADRÃO
// ============================================================

MuonHit::MuonHit()
  : EDM(),
    m_sampling(MuonSampling::Inner),
    m_detector(MuonChamberType::MDT),
    m_region(MuonRegion::Barrel),
    m_x(0),
    m_y(0),
    m_z(0),
    m_dx(0),
    m_dy(0),
    m_hash(0),
    m_bcid_start(0),
    m_bcid_end(0),
    m_bc_duration(25.0f),
    m_firstHit(false),
    m_energyDeposit(0.0f),
    m_timeValue(0.0f)
{
    int total = m_bcid_end - m_bcid_start + 1;

    m_time.reserve(total);

    float start = (m_bcid_start - 0.5f) * m_bc_duration;
    float step  = m_bc_duration;

    for (int i = 0; i < total; ++i) {
        m_time.push_back(start + step * i);
    }
}


// ============================================================
// CONSTRUTOR PRINCIPAL
// ============================================================

MuonHit::MuonHit(
    float x,
    float y,
    float z,
    float dx,
    float dy,
    unsigned long int hash,
    MuonSampling sampling,
    MuonChamberType detector,
    MuonRegion region,
    float bc_duration,
    int bcid_start,
    int bcid_end,
    float energy)
  : EDM(),
    m_sampling(sampling),
    m_detector(detector),
    m_region(region),
    m_x(x),
    m_y(y),
    m_z(z),
    m_dx(dx),
    m_dy(dy),
    m_hash(hash),
    m_bcid_start(bcid_start),
    m_bcid_end(bcid_end),
    m_bc_duration(bc_duration),
    m_firstHit(false),
    m_energyDeposit(energy),
    m_timeValue(0.0f)
{
    static std::set<std::tuple<int,int,int>> seen;
    auto key = std::make_tuple(static_cast<int>(sampling),
                                static_cast<int>(detector),
                                static_cast<int>(region));
    if (seen.find(key) == seen.end()) {
        seen.insert(key);
        std::cout << "DEBUG MuonHit constructor: sampling="
                  << static_cast<int>(sampling)
                  << ", detector=" << static_cast<int>(detector)
                  << ", region=" << static_cast<int>(region)
                  << ", energy=" << energy << " MeV"
                  << std::endl;
    }

    int total = m_bcid_end - m_bcid_start + 1;

    m_time.reserve(total);

    float start = (m_bcid_start - 0.5f) * m_bc_duration;
    float step  = m_bc_duration;

    for (int i = 0; i < total; ++i) {
        m_time.push_back(start + step * i);
    }
}


// ============================================================
// CLEAR
// ============================================================

void MuonHit::clear()
{
    m_edep.clear();
    m_tof.clear();
    m_driftTime.clear();

    m_firstHit = false;
    m_energyDeposit = 0.0f;
    m_timeValue = 0.0f;

    m_pulse.clear();
}


// ============================================================
// ENERGIA TOTAL
// ============================================================

float MuonHit::energyDeposit() const
{
    if (m_energyDeposit > 0.0f) {
        return m_energyDeposit;
    }

    float total = 0.0f;
    for (const auto& pair : m_edep) {
        total += pair.second;
    }
    return total;
}


// ============================================================
// FILL SEM RUÍDO  (THRESHOLD POR DETECTOR + DIAGNÓSTICO DE TEMPO)
// ============================================================

void MuonHit::fill(const G4Step* step)
{
    // Energia depositada no step (em GeV no Geant4)
    float edep = (float)step->GetTotalEnergyDeposit();

    // Converte a energia para MeV
    float edep_mev = edep / MeV;

    G4StepPoint* point = step->GetPreStepPoint();

    float t = (float)point->GetGlobalTime() / ns;

    // ============================================================
    // DIAGNÓSTICO DE TEMPO (primeiros 50 steps)
    // ------------------------------------------------------------
    // Objetivo: confirmar se find(t) == -1 está descartando hits
    // de RPC/TGC (region 1/2) por estarem fora da janela de BCID.
    // ============================================================
    static int time_debug = 0;
    if (time_debug < 50) {
        int find_result = find(t);
        std::cout
            << "\n[MUON TIME DEBUG]"
            << "\n  Detector = "         << static_cast<int>(m_detector)
            << "\n  Region = "           << static_cast<int>(m_region)
            << "\n  Sampling = "         << static_cast<int>(m_sampling)
            << "\n  Edep MeV = "         << edep_mev
            << "\n  Global time ns = "   << t
            << "\n  BCID start = "       << m_bcid_start
            << "\n  BCID end = "         << m_bcid_end
            << "\n  bc_duration = "      << m_bc_duration
            << "\n  m_time size = "      << m_time.size()
            << "\n  m_time first = "     << (m_time.empty() ? -999 : m_time.front())
            << "\n  m_time last = "      << (m_time.empty() ? -999 : m_time.back())
            << "\n  find(t) = "          << find_result
            << std::endl;
        ++time_debug;
    }

    // ---- FILTRO FÍSICO: threshold específico do detector ----
    const float kThresholdMeV = thresholdForDetector(m_detector);
    if (edep_mev < kThresholdMeV) {
        return;
    }

    // ---- FILTRO TEMPORAL ----
    int samp = find(t);
    if (samp == -1)
        return;

    int bcid = m_bcid_start + samp;

    // Acumula no mapa
    m_edep[bcid] += edep_mev;

    // Acumula no total
    m_energyDeposit += edep_mev;

    // ============================================================
    // DEBUG (apenas para steps acima de kDebugThresholdMeV = 1 keV)
    // ============================================================
    if (edep_mev > kDebugThresholdMeV) {

        static int debug_count = 0;
        static const int MAX_DEBUG = 10;

        if (debug_count < MAX_DEBUG) {
            debug_count++;
            std::cout << "\n!!! FILL DEBUG [" << debug_count << "] !!!" << std::endl;
            std::cout << "  Step energy: " << edep_mev << " MeV" << std::endl;
            std::cout << "  Time: " << t << " ns" << std::endl;
            std::cout << "  samp=" << samp << ", bcid=" << bcid << std::endl;
            std::cout << "  m_edep[bcid] antes = " << (m_edep[bcid] - edep_mev) << std::endl;
            std::cout << "  m_energyDeposit antes = " << (m_energyDeposit - edep_mev) << std::endl;
            std::cout << "  m_edep[bcid] depois = " << m_edep[bcid] << std::endl;
            std::cout << "  m_energyDeposit depois = " << m_energyDeposit << std::endl;
            std::cout << "  Sampling: " << static_cast<int>(m_sampling) << std::endl;
            std::cout << "  Detector: " << static_cast<int>(m_detector) << std::endl;
            std::cout << "  Region: " << static_cast<int>(m_region) << std::endl;
            std::cout << "  Pos: (" << m_x << ", " << m_y << ", " << m_z << ")" << std::endl;
            std::cout << "  Threshold aplicado: " << kThresholdMeV*1000.0f << " keV" << std::endl;
            std::cout << "!!! FIM DEBUG !!!\n" << std::endl;
        }

        static int z_debug_count = 0;
        static const int MAX_Z_DEBUG = 100;

        G4ThreeVector position = point->GetPosition();
        float z_position = (float)position.z() / mm;

        if (z_position < 0.0f && z_debug_count < MAX_Z_DEBUG) {
            z_debug_count++;
            std::cout << "\n!!! Z NEGATIVO DEBUG [" << z_debug_count << "/" << MAX_Z_DEBUG << "] !!!" << std::endl;
            std::cout << "  Step Z position: " << z_position << " mm" << std::endl;
            std::cout << "  Step energy: " << edep_mev << " MeV" << std::endl;
            std::cout << "  Time: " << t << " ns" << std::endl;
            std::cout << "  Hit Z position (m_z): " << m_z << " mm" << std::endl;
            std::cout << "  Hit X position (m_x): " << m_x << " mm" << std::endl;
            std::cout << "  Hit Y position (m_y): " << m_y << " mm" << std::endl;
            std::cout << "  Detector: " << static_cast<int>(m_detector) << std::endl;
            std::cout << "  Sampling: " << static_cast<int>(m_sampling) << std::endl;
            std::cout << "  Region: " << static_cast<int>(m_region) << std::endl;
            std::cout << "  bcid: " << bcid << std::endl;
            std::cout << "  Threshold: " << kThresholdMeV*1000.0f << " keV" << std::endl;
            std::cout << "  Step position (x,y,z): ("
                      << position.x()/mm << ", "
                      << position.y()/mm << ", "
                      << position.z()/mm << ") mm" << std::endl;
            std::cout << "!!! FIM Z NEGATIVO DEBUG !!!\n" << std::endl;
        }
    }

    // ============================================================
    // TRATAMENTO ESPECÍFICO POR TIPO DE DETECTOR
    // ============================================================
    G4ThreeVector position = point->GetPosition();

    if (m_detector == MuonChamberType::RPC)
    {
        if (!m_firstHit && m_edep[bcid] > 0.0f)
        {
            m_tof[bcid] = t;
            m_firstHit = true;
        }
    }
    else if (m_detector == MuonChamberType::MDT)
    {
        m_tof[bcid] = t;

        float drift_distance =
            std::sqrt(position.x()*position.x()
                    + position.y()*position.y());

        float drift_velocity = 0.05f;

        m_driftTime[bcid] =
            drift_distance / drift_velocity;
    }
    else if (m_detector == MuonChamberType::TGC)
    {
        m_tof[bcid] = t;
    }
}


// ============================================================
// FILL COM RUÍDO
// ============================================================

void MuonHit::fill(const G4Step* step,
                   float sampNoiseStd)
{
    // fill(step) já aplica threshold + filtro temporal + debug de tempo
    fill(step);

    // Energia do step (mesma conversão)
    float edep_mev = (float)step->GetTotalEnergyDeposit() / MeV;

    // Se foi descartado pelo threshold, não continua
    const float kThresholdMeV = thresholdForDetector(m_detector);
    if (edep_mev < kThresholdMeV) {
        return;
    }

    G4StepPoint* point = step->GetPreStepPoint();

    float t = (float)point->GetGlobalTime() / ns;

    int samp = find(t);

    if (samp == -1)
        return;

    int bcid = m_bcid_start + samp;

    // Supressão de ruído em RPC
    if (m_detector == MuonChamberType::RPC
        && !m_firstHit)
    {
        if (m_edep[bcid] <= sampNoiseStd / MeV)
        {
            m_tof[bcid] = 0.0f;
        }
    }
}


// ============================================================
// FIND  (BUG CORRIGIDO)
// ------------------------------------------------------------
// Antes: if (index >= (int)m_time.size() - 1) return -1;
//        rejeitava a ÚLTIMA borda da janela.
// Agora: if (index >= (int)m_time.size())     return -1;
//        só rejeita se estiver FORA da janela.
// ============================================================

int MuonHit::find(float value) const
{
    auto it =
      std::lower_bound(m_time.begin(),
                       m_time.end(),
                       value);

    if (it == m_time.end())
        return -1;

    int index =
      std::distance(m_time.begin(), it);

    // CORRIGIDO: só rejeita se estiver fora da janela
    if (index >= (int)m_time.size())
        return -1;

    return index;
}


// ============================================================
// ACCESSORS
// ============================================================

float MuonHit::edep(int bcid) const
{
    auto it = m_edep.find(bcid);
    if (it != m_edep.end())
        return it->second;
    return 0.0f;
}


float MuonHit::tof(int bcid) const
{
    auto it = m_tof.find(bcid);
    if (it != m_tof.end())
        return it->second;
    return 0.0f;
}


float MuonHit::driftTime(int bcid) const
{
    auto it = m_driftTime.find(bcid);
    if (it != m_driftTime.end())
        return it->second;
    return 0.0f;
}


void MuonHit::set_edep(int bcid, float value)      { m_edep[bcid] = value; }
void MuonHit::set_tof(int bcid, float value)       { m_tof[bcid] = value; }
void MuonHit::set_driftTime(int bcid, float value) { m_driftTime[bcid] = value; }


bool MuonHit::firstHit() const
{
    return m_firstHit;
}
