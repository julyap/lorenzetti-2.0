// ============================================================================
// Arquivo: MuonHit/src/MuonHitConverter.cxx
// Descrição: Implementação da classe xAOD::MuonHitConverter.
//            - convert(const MuonHit*, MuonHit_t&): serializa todos os
//              campos básicos e itera sobre o range de BCIDs para preencher
//              os vetores de mapa (apenas BCIDs com edep > 0), preservando
//              a evolução temporal do sinal.
//            - convert(const MuonHit_t&, MuonHit*&): cria um novo
//              xAOD::MuonHit com os campos básicos e restaura os mapas de
//              BCID usando set_edep/set_tof/set_driftTime.
// ============================================================================

#include "MuonHit/MuonHitConverter.h"
#include <iostream>

using namespace xAOD;

//!=====================================================================
// Serialização: MuonHit* → MuonHit_t
//   - Copia todos os campos básicos
//   - Itera sobre os BCIDs e, para cada um com edep > 0, adiciona os
//     valores correspondentes de bcid/edep/tof/driftTime nos vetores
//!=====================================================================
bool MuonHitConverter::convert(const MuonHit *hit, MuonHit_t &hit_t)
{
  if (!hit) return false;

  // Campos básicos
  hit_t.sampling    = static_cast<int>(hit->sampling());
  hit_t.detector    = static_cast<int>(hit->detector());
  hit_t.region      = static_cast<int>(hit->region());
  hit_t.x           = hit->x();
  hit_t.y           = hit->y();
  hit_t.z           = hit->z();
  hit_t.dx          = hit->dx();
  hit_t.dy          = hit->dy();
  hit_t.bcid_start  = hit->bcid_start();
  hit_t.bcid_end    = hit->bcid_end();
  hit_t.bc_duration = hit->bc_duration();
  hit_t.hash        = hit->hash();
  hit_t.energy      = hit->energyDeposit();
  hit_t.time        = hit->time();

  // ============================================
  // SALVAR MAPAS DE BCID
  // ============================================
  hit_t.bcid_values.clear();
  hit_t.edep_values.clear();
  hit_t.tof_values.clear();
  hit_t.driftTime_values.clear();

  for (int bcid = hit->bcid_start(); bcid <= hit->bcid_end(); ++bcid) {
    float edep = hit->edep(bcid);
    if (edep > 0) {
      hit_t.bcid_values.push_back(bcid);
      hit_t.edep_values.push_back(edep);
      hit_t.tof_values.push_back(hit->tof(bcid));
      hit_t.driftTime_values.push_back(hit->driftTime(bcid));
    }
  }

  // Debug: mostrar informação completa
  static int debug_count = 0;
  if (hit_t.energy > 0.000001f && debug_count < 10) {
    debug_count++;
    std::cout << "\n=== CONVERTER DEBUG (SAVE) ===" << std::endl;
    std::cout << "  Energia: " << hit_t.energy << " MeV" << std::endl;
    std::cout << "  Sampling: " << hit_t.sampling
              << ", Detector: " << hit_t.detector
              << ", Region: " << hit_t.region << std::endl;
    std::cout << "  Pos: (" << hit_t.x << ", " << hit_t.y << ", " << hit_t.z << ")" << std::endl;
    std::cout << "  BCID: [" << hit_t.bcid_start << ", " << hit_t.bcid_end
              << "], Duration: " << hit_t.bc_duration << " ns" << std::endl;
    std::cout << "  BCIDs com energia: " << hit_t.bcid_values.size() << std::endl;
    for (size_t i = 0; i < hit_t.bcid_values.size() && i < 5; ++i) {
      std::cout << "    BCID " << hit_t.bcid_values[i]
                << ": edep=" << hit_t.edep_values[i] << " MeV"
                << ", tof=" << hit_t.tof_values[i] << " ns" << std::endl;
    }
    std::cout << "==============================\n" << std::endl;
  }

  return true;
}

//!=====================================================================
// Desserialização: MuonHit_t → MuonHit*
//   - Cria um novo MuonHit com os campos básicos e a energia total
//   - Restaura os mapas de BCID usando set_edep/set_tof/set_driftTime
//!=====================================================================
bool MuonHitConverter::convert(const MuonHit_t &hit_t, MuonHit *&hit)
{
  // ============================================
  // CRIAR HIT COM ENERGIA TOTAL
  // ============================================
  hit = new xAOD::MuonHit(
    hit_t.x, hit_t.y, hit_t.z,
    hit_t.dx, hit_t.dy,
    hit_t.hash,
    static_cast<MuonSampling>(hit_t.sampling),
    static_cast<MuonChamberType>(hit_t.detector),
    static_cast<MuonRegion>(hit_t.region),
    hit_t.bc_duration,
    hit_t.bcid_start,
    hit_t.bcid_end,
    hit_t.energy
  );

  hit->setTime(hit_t.time);

  // ============================================
  // CARREGAR MAPAS DE BCID
  // ============================================
  for (size_t i = 0; i < hit_t.bcid_values.size(); ++i) {
    int bcid = hit_t.bcid_values[i];
    hit->set_edep(bcid, hit_t.edep_values[i]);
    hit->set_tof(bcid, hit_t.tof_values[i]);
    hit->set_driftTime(bcid, hit_t.driftTime_values[i]);
  }

  // Debug: mostrar informação completa
  static int debug_count = 0;
  if (hit_t.energy > 0.000001f && debug_count < 10) {
    debug_count++;
    std::cout << "\n=== CONVERTER DEBUG (LOAD) ===" << std::endl;
    std::cout << "  Energia carregada: " << hit_t.energy << " MeV" << std::endl;
    std::cout << "  Sampling: " << hit_t.sampling
              << ", Detector: " << hit_t.detector
              << ", Region: " << hit_t.region << std::endl;
    std::cout << "  Tempo: " << hit_t.time << " ns" << std::endl;
    std::cout << "  BCIDs carregados: " << hit_t.bcid_values.size() << std::endl;
    for (size_t i = 0; i < hit_t.bcid_values.size() && i < 5; ++i) {
      std::cout << "    BCID " << hit_t.bcid_values[i]
                << ": edep=" << hit_t.edep_values[i] << " MeV"
                << ", tof=" << hit_t.tof_values[i] << " ns" << std::endl;
    }
    std::cout << "==============================\n" << std::endl;
  }

  return true;
}
