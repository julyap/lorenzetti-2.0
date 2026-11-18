// ============================================================================
// Arquivo: MuonHit/src/MuonHit.cxx
// Autora: Julya Pacheco Ferreira
// Descrição: Implementação da classe xAOD::MuonHit.
//            - Construtores: default e principal (inicializam os vetores
//              internos de tempo por BCID).
//            - clear(): limpa mapas, flags e pulso.
//            - energyDeposit(): retorna o acumulador rápido (se > 0) ou a
//              soma do mapa de edep.
//            - fill(G4Step): preenche o hit a partir de um step, acumulando
//              energia no BCID correspondente e tratando diferentes tipos de
//              detector (RPC, MDT, TGC) para tof/driftTime.
//            - fill(G4Step, sampNoiseStd): variante com supressão de ruído
//              em RPC.
//            - find(): encontra o índice do BCID correspondente a um tempo.
//            - Accessors por BCID (edep, tof, driftTime) e setters
//              correspondentes.
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

    // Pré-calcula o vetor de tempos: (bcid - 0.5) * bc_duration
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
    // DEBUG SELETIVO (apenas 1x por combinação sampling/detector/region)
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

    // Pré-calcula o vetor de tempos por BCID
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
// ENERGIA TOTAL - CORREÇÃO PRINCIPAL!
// ============================================================

float MuonHit::energyDeposit() const
{
    // Se o acumulador já foi calculado (durante simulação), usa ele
    if (m_energyDeposit > 0.0f) {
        return m_energyDeposit;
    }

    // Durante leitura do arquivo, calcula somando o mapa
    float total = 0.0f;
    for (const auto& pair : m_edep) {
        total += pair.second;
    }
    return total;
}


// ============================================================
// FILL SEM RUÍDO
// ============================================================

void MuonHit::fill(const G4Step* step)
{
    // Energia depositada no step (em GeV no Geant4)
    float edep = (float)step->GetTotalEnergyDeposit();

    G4StepPoint* point = step->GetPreStepPoint();

    // Tempo global do step, convertido para ns
    float t = (float)point->GetGlobalTime() / ns;

    // Encontra o índice do BCID correspondente
    int samp = find(t);

    if (samp == -1)
        return;

    int bcid = m_bcid_start + samp;

    // Converte a energia para MeV
    float edep_mev = edep / MeV;

    // Acumula no mapa
    m_edep[bcid] += edep_mev;

    // Acumula no total também (para uso rápido durante simulação)
    m_energyDeposit += edep_mev;

    // DEBUG para verificar energia durante simulação
    static int debug_count = 0;
    static const int MAX_DEBUG = 10;

    if (edep_mev > 0.000001f && debug_count < MAX_DEBUG) {
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
        std::cout << "!!! FIM DEBUG !!!\n" << std::endl;
    }

    // DEBUG para mostrar valores de Z negativos (apenas primeiros 100)
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
        std::cout << "  Step position (x,y,z): ("
                  << position.x()/mm << ", "
                  << position.y()/mm << ", "
                  << position.z()/mm << ") mm" << std::endl;
        std::cout << "!!! FIM Z NEGATIVO DEBUG !!!\n" << std::endl;
    }

    // Tratamento específico por tipo de detector
    if (m_detector == MuonChamberType::RPC)
    {
        // RPC: registra o tof do primeiro hit com energia > 0
        if (!m_firstHit && m_edep[bcid] > 0.0f)
        {
            m_tof[bcid] = t;
            m_firstHit = true;
        }
    }
    else if (m_detector == MuonChamberType::MDT)
    {
        // MDT: tof = tempo; driftTime = distância radial / drift velocity
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
        // TGC: apenas tof
        m_tof[bcid] = t;
    }
}


// ============================================================
// FILL COM RUÍDO
// ============================================================

void MuonHit::fill(const G4Step* step,
                   float sampNoiseStd)
{
    // Preenche primeiro (sem ruído)
    fill(step);

    G4StepPoint* point = step->GetPreStepPoint();

    float t = (float)point->GetGlobalTime() / ns;

    int samp = find(t);

    if (samp == -1)
        return;

    int bcid = m_bcid_start + samp;

    // Supressão de ruído em RPC: se a energia for menor que o ruído,
    // zera o tof
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
// FIND
// ============================================================

int MuonHit::find(float value) const
{
    // Encontra o primeiro tempo >= value
    auto it =
      std::lower_bound(m_time.begin(),
                       m_time.end(),
                       value);

    if (it == m_time.end())
        return -1;

    int index =
      std::distance(m_time.begin(), it);

    // Se estiver na última borda, considera fora
    if (index >= (int)m_time.size() - 1)
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


void MuonHit::set_edep(int bcid,
                       float value)
{
    m_edep[bcid] = value;
}


void MuonHit::set_tof(int bcid,
                      float value)
{
    m_tof[bcid] = value;
}


void MuonHit::set_driftTime(int bcid,
                            float value)
{
    m_driftTime[bcid] = value;
}


bool MuonHit::firstHit() const
{
    return m_firstHit;
}
