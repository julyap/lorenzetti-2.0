// ============================================================================
// Arquivo: src/MuonChamberConverter.cxx
// Autora: Julya Pacheco Ferreira
// Descrição: Implementação da classe xAOD::MuonChamberConverter, responsável
//            por converter entre:
//              - MuonChamberCell* ↔ MuonChamber_t (serialização/desserialização
//                de células de múon para persistência em ROOT);
//              - MuonChamberDescriptor_t → MuonChamberCell* (reconstrução de
//                células a partir de descriptors persistidos).
//            Também fornece utilitários estáticos para cálculo de eta e phi
//            a partir de (x, y, z).
// ============================================================================

#include "MuonCell/MuonChamberConverter.h"
#include "GaugiKernel/EDM.h"
#include <cmath>

using namespace xAOD;

//==============================================================================
// Serialização: MuonChamberCell* → MuonChamber_t
//   - Copia posição/dimensões, sinal (amplitude, tempo), identificação
//     (isMuon, theta, phi), hash e link para o descriptor
//   - Copia campos adicionais (eta, deltaEta, deltaPhi, energy, trackLength,
//     recoTime, signal, efficiency, isTrigger, layer)
//   - Copia os campos do descriptor (sampling, detector/chamberType, region,
//     layer); se não houver descriptor, zera todos
//==============================================================================
bool MuonChamberConverter::convert( const MuonChamberCell *cell, MuonChamber_t &cell_t)
{
  if(cell){
      // Posição e dimensões
      cell_t.x        = cell->x();
      cell_t.y        = cell->y();
      cell_t.z        = cell->z();
      cell_t.dx       = cell->dx();
      cell_t.dy       = cell->dy();
      cell_t.dz       = cell->dz();

      // Propriedades do sinal
      cell_t.amplitude = cell->amplitude();
      cell_t.time      = cell->time();

      // Identificação de múon
      cell_t.isMuon    = cell->isMuon();
      cell_t.theta     = cell->theta();
      cell_t.phi       = cell->phi();

      // Hash e link para descriptor
      cell_t.hash            = 0;
      cell_t.descriptor_link = cell->descriptor() ? cell->descriptor()->hash() : 0;

      // Campos adicionais
      cell_t.eta         = cell->m_eta;
      cell_t.deltaEta    = cell->m_deltaEta;
      cell_t.deltaPhi    = cell->m_deltaPhi;
      cell_t.energy      = cell->m_energy;
      cell_t.trackLength = cell->m_trackLength;
      cell_t.recoTime    = cell->m_recoTime;
      cell_t.signal      = cell->m_signal;
      cell_t.efficiency  = cell->m_efficiency;
      cell_t.isTrigger   = cell->m_isTrigger;
      cell_t.layer       = cell->m_layer;

      // ===== CAMPOS DO DESCRIPTOR =====
      if (cell->descriptor()) {
          cell_t.sampling = cell->descriptor()->sampling();
          cell_t.detector = cell->descriptor()->chamberType();
          cell_t.chamberType = cell->descriptor()->chamberType();
          cell_t.region   = cell->descriptor()->region();
          cell_t.layer    = cell->descriptor()->layer();
      } else {
          cell_t.sampling    = 0;
          cell_t.detector    = 0;
          cell_t.chamberType = 0;
          cell_t.region      = 0;
          cell_t.layer       = 0;
      }

      return true;
  }
  return false;
}

//==============================================================================
// Desserialização: MuonChamber_t → MuonChamberCell*
//   - Cria uma nova célula com posição/dimensões
//   - Preenche sinal (amplitude, tempo), identificação (isMuon, theta, phi)
//   - Preenche campos adicionais (eta, deltaEta, deltaPhi, energy,
//     trackLength, recoTime, signal, efficiency, isTrigger, layer)
//   - O descriptor é definido como nullptr (será reatribuído depois, se
//     necessário, pelo código que chama esta função)
//==============================================================================
bool MuonChamberConverter::convert( const MuonChamber_t &cell_t, MuonChamberCell *&cell )
{
  // Cria nova célula
  cell = new xAOD::MuonChamberCell( cell_t.x,
                                    cell_t.y,
                                    cell_t.z,
                                    cell_t.dx,
                                    cell_t.dy,
                                    cell_t.dz );

  if (!cell) return false;

  // Define propriedades do sinal
  cell->setAmplitude(cell_t.amplitude);
  cell->setTime(cell_t.time);

  // Define identificação de múon
  cell->setIsMuon(cell_t.isMuon);
  cell->setTheta(cell_t.theta);
  cell->setPhi(cell_t.phi);

  // Define campos adicionais
  cell->m_eta         = cell_t.eta;
  cell->m_deltaEta    = cell_t.deltaEta;
  cell->m_deltaPhi    = cell_t.deltaPhi;
  cell->m_energy      = cell_t.energy;
  cell->m_trackLength = cell_t.trackLength;
  cell->m_recoTime    = cell_t.recoTime;
  cell->m_signal      = cell_t.signal;
  cell->m_efficiency  = cell_t.efficiency;
  cell->m_isTrigger   = cell_t.isTrigger;
  cell->m_layer       = cell_t.layer;


  cell->setDescriptor(nullptr);

  return true;
}

//==============================================================================
// Reconstrução: MuonChamberDescriptor_t → MuonChamberCell*
//   - Cria célula a partir da posição/dimensões do descriptor
//   - Usa energy como amplitude e driftTime como tempo
//   - Marca isMuon=true
//   - Calcula theta a partir do eta e copia phi
//   - Copia eta, deltaEta, deltaPhi, truthEnergy → m_energy, efficiency,
//     layer
//   - isTrigger = (chamberType == 1 || chamberType == 2) (RPC/TGC)
//   - O descriptor é definido como nullptr
//==============================================================================
bool MuonChamberConverter::convert( const MuonChamberDescriptor_t &desc_t,
                                   MuonChamberCell *&cell )
{
    // Criar célula com posição e dimensões
    cell = new xAOD::MuonChamberCell(
        desc_t.x, desc_t.y, desc_t.z,
        desc_t.dx, desc_t.dy, desc_t.dz
    );

    if (!cell) return false;

    // SINAL
    cell->setAmplitude(desc_t.energy);
    cell->setTime(desc_t.driftTime);

    // ===== IDENTIFICAÇÃO =====
    cell->setIsMuon(true);

    float theta = 2.0f * std::atan(std::exp(-desc_t.eta));
    cell->setTheta(theta);
    cell->setPhi(desc_t.phi);

    // ===== CAMPOS ADICIONAIS =====
    cell->m_eta = desc_t.eta;
    cell->m_deltaEta = desc_t.deltaEta;
    cell->m_deltaPhi = desc_t.deltaPhi;
    cell->m_energy = desc_t.truthEnergy;
    cell->m_trackLength = 0.0f;
    cell->m_recoTime = desc_t.driftTime;
    cell->m_signal = desc_t.energy;
    cell->m_efficiency = desc_t.efficiency;
    cell->m_isTrigger = (desc_t.chamberType == 1 || desc_t.chamberType == 2);
    cell->m_layer = desc_t.layer;


    cell->setDescriptor(nullptr);

    return true;
}

//==============================================================================
// Utilitário: cálculo de eta a partir de (x, y, z)
//==============================================================================
float MuonChamberConverter::calcEta(float x, float y, float z)
{
    float rho = std::sqrt(x*x + y*y);
    if (std::abs(z) < 0.0001f) return 0.0f;
    float theta = std::atan2(rho, std::abs(z));
    return (z >= 0 ? 1 : -1) * std::log(std::tan(theta/2.0f));
}

//==============================================================================
// Utilitário: cálculo de phi a partir de (x, y)
//==============================================================================
float MuonChamberConverter::calcPhi(float x, float y)
{
    float phi = std::atan2(y, x);
    if (phi < 0) phi += 2 * M_PI;
    return phi;
} 
