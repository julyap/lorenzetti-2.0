// ============================================================================
// Arquivo: MuonHitMerge.cxx
// Descrição: Implementação do algoritmo MuonHitMerge. Lê todas as coleções
//            de hits de muon (MuonHitCollection) produzidas pelos múltiplos
//            MuonHitMaker e as funde em uma única coleção de hits
//            (OutputHitsKey), copiando todas as propriedades relevantes:
//            posição/tamanho, energia, tempo, pulso e mapas por BCID
//            (edep, tof, driftTime). Também emite estatísticas detalhadas
//            (energia total, mín./máx./média, energia por sampling).
// ============================================================================

#include "MuonHit/MuonHitContainer.h"
#include "MuonHit/MuonHitCollection.h"

#include "MuonHitMerge.h"
#include "TVector3.h"
#include <cstdlib>

#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH2Poly.h"


using namespace Gaugi;
using namespace SG;


MuonHitMerge::MuonHitMerge( std::string name ) :
  IMsgService(name),
  Algorithm()
{
  // --- Propriedades configuráveis ---------------------------------
  declareProperty( "InputCollectionKeys"    , m_collectionKeys={}           );
  declareProperty( "OutputHitsKey"          , m_hitsKey="MuonHits"          );
  declareProperty( "OutputLevel"            , m_outputLevel=1               );
}

//!=====================================================================

MuonHitMerge::~MuonHitMerge()
{;}

//!=====================================================================

StatusCode MuonHitMerge::initialize()
{
  CHECK_INIT();
  setMsgLevel( m_outputLevel );
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMerge::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMerge::bookHistograms( SG::EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMerge::pre_execute( EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMerge::execute( EventContext &/*ctx*/ , const G4Step * /*step*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMerge::execute( EventContext &ctx, int /*evt*/ ) const
{
  return post_execute(ctx);
}

//!=====================================================================

StatusCode MuonHitMerge::post_execute( EventContext &ctx ) const
{
  MSG_DEBUG( "Starting muon hit collection merge algorithm..." );
  MSG_DEBUG( "Creating MuonHIT containers with key " << m_hitsKey);

  // Cria o container de saída
  SG::WriteHandle<xAOD::MuonHitContainer> container( m_hitsKey , ctx );
  container.record( std::unique_ptr<xAOD::MuonHitContainer>(new xAOD::MuonHitContainer()) );

  // Estatísticas
  float etot = 0;
  int totalHits = 0;
  int hitsWithEnergy = 0;
  float maxEnergy = 0;
  float minEnergy = 999999;

  std::map<int, float> energyBySampling;
  std::map<int, int> hitsBySampling;

  // --- Loop sobre as coleções de entrada --------------------------
  for ( auto key : m_collectionKeys ){

    MSG_DEBUG( "Reading all hits from muon collection with key " << key );
    SG::ReadHandle<xAOD::MuonHitCollection> collection( key, ctx );

    if( !collection.isValid() ){
      MSG_WARNING( "It's not possible to read the xAOD::MuonHitCollection from this Context using this key: " << key );
      continue;
    }

    for (const auto &pair : **collection.ptr() )
    {
      const xAOD::MuonHit* const_hit = pair.second;

      // ============================================================
      // CORREÇÃO PRINCIPAL: Usar a energia que já está correta!
      // O const_hit já tem a energia acumulada durante o fill
      // ============================================================
      float energy = const_hit->energyDeposit();

      // Criar novo hit com a energia correta
      auto hit = new xAOD::MuonHit(
        const_hit->x(), const_hit->y(), const_hit->z(),
        const_hit->dx(), const_hit->dy(), const_hit->hash(),
        const_hit->sampling(),
        const_hit->detector(),
        const_hit->region(),
        const_hit->bc_duration(),
        const_hit->bcid_start(),
        const_hit->bcid_end(),
        energy  // ← Energia correta!
      );

      // Copiar o tempo e o pulso
      hit->setTime(const_hit->time());

      if (!const_hit->pulse().empty()) {
        hit->setPulse(const_hit->pulse());
      }

      // ============================================================
      // COPIAR OS MAPAS DE ENERGIA POR BCID (CRUCIAL!)
      // ============================================================
      // Copiar o mapa de edep (energia por BCID)
      for (int bcid = const_hit->bcid_start(); bcid <= const_hit->bcid_end(); ++bcid) {
        float edep_value = const_hit->edep(bcid);
        if (edep_value > 0) {
          hit->set_edep(bcid, edep_value);
        }
      }

      // Copiar o mapa de tof (tempo de voo)
      for (int bcid = const_hit->bcid_start(); bcid <= const_hit->bcid_end(); ++bcid) {
        float tof_value = const_hit->tof(bcid);
        if (tof_value > 0) {
          hit->set_tof(bcid, tof_value);
        }
      }

      // Copiar o mapa de driftTime
      for (int bcid = const_hit->bcid_start(); bcid <= const_hit->bcid_end(); ++bcid) {
        float drift_value = const_hit->driftTime(bcid);
        if (drift_value > 0) {
          hit->set_driftTime(bcid, drift_value);
        }
      }

      // Copiar firstHit flag
      if (const_hit->firstHit()) {
        MSG_DEBUG( "First hit detected for hash " << const_hit->hash() );
      }

      // ============================================================
      // ESTATÍSTICAS
      // ============================================================
      float new_energy = hit->energyDeposit();
      etot += new_energy;
      totalHits++;

      if (new_energy > 0) {
          hitsWithEnergy++;
          if (new_energy > maxEnergy) maxEnergy = new_energy;
          if (new_energy < minEnergy) minEnergy = new_energy;

          int sampling = static_cast<int>(hit->sampling());
          energyBySampling[sampling] += new_energy;
          hitsBySampling[sampling]++;
      }

      // Debug para os primeiros hits com energia
      if (hitsWithEnergy <= 10 && new_energy > 0) {
        MSG_INFO( "HIT #" << hitsWithEnergy
                 << ": sampling=" << static_cast<int>(hit->sampling())
                 << ", detector=" << static_cast<int>(hit->detector())
                 << ", region=" << static_cast<int>(hit->region())
                 << ", energy=" << new_energy << " MeV"
                 << ", x=" << hit->x() << ", y=" << hit->y() << ", z=" << hit->z());

        // Debug dos BCIDs
        MSG_INFO( "  BCIDs: start=" << hit->bcid_start()
                 << ", end=" << hit->bcid_end()
                 << ", duration=" << hit->bc_duration() );

        // Mostrar os BCIDs que têm energia
        for (int bcid = hit->bcid_start(); bcid <= hit->bcid_end(); ++bcid) {
          float e = hit->edep(bcid);
          if (e > 0) {
            MSG_INFO( "    BCID " << bcid << ": edep=" << e << " MeV, tof=" << hit->tof(bcid) );
          }
        }
      }

      container->push_back( hit );
    }
  }

  // ============================================================
  // RELATÓRIO FINAL
  // ============================================================
  MSG_INFO( "===========================================" );
  MSG_INFO( "MUON HIT MERGE - ESTATÍSTICAS DETALHADAS" );
  MSG_INFO( "===========================================" );
  MSG_INFO( "Total de hits processados: " << totalHits );
  MSG_INFO( "Hits com energia > 0: " << hitsWithEnergy );
  MSG_INFO( "Hits sem energia (0): " << (totalHits - hitsWithEnergy) );

  if (hitsWithEnergy > 0) {
      MSG_INFO( "Energia mínima: " << minEnergy << " MeV" );
      MSG_INFO( "Energia máxima: " << maxEnergy << " MeV" );
      MSG_INFO( "Energia média: " << (etot/hitsWithEnergy) << " MeV" );
  }

  MSG_INFO( "-------------------------------------------" );
  MSG_INFO( "Energia total: " << etot << " MeV" );
  MSG_INFO( "-------------------------------------------" );

  if (!energyBySampling.empty()) {
      MSG_INFO( "Energia por sampling:" );
      for (auto& pair : energyBySampling) {
          MSG_INFO( "  Sampling " << pair.first
                   << ": " << pair.second << " MeV"
                   << " (" << hitsBySampling[pair.first] << " hits)");
      }
  }

  MSG_INFO( "===========================================" );

  // Verificar se os dados foram salvos corretamente
  MSG_INFO( "Container size: " << container->size() );
  MSG_INFO( "Hits with valid edep map: " << hitsWithEnergy );

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMerge::fillHistograms( SG::EventContext &ctx ) const
{
  MSG_DEBUG("fillHistograms called, but no histograms are booked for MuonHitMerge.");
  return StatusCode::SUCCESS;
}
