// ============================================================================
// Arquivo: MuonCellMerge.cxx
// Descrição: Implementação do algoritmo MuonCellMerge. Lê todas as coleções
//            de descriptors de células de muon (MuonChamberDetDescriptorCollection)
//            produzidas pelos múltiplos MuonCellMaker e as funde em:
//              - uma única coleção de células reconstruídas (OutputCellsKey),
//                aplicando eficiência (simulação de perda de detecção);
//              - uma única coleção de células de verdade (OutputTruthCellsKey),
//                com a energia total depositada (soma de edep por BCID).
//            Evita duplicatas via conjunto de hashes já processados.
// ============================================================================

#include "MuonCell/MuonChamberCellContainer.h"
#include "MuonCell/MuonChamberDetDescriptorCollection.h"
#include "MuonCellMerge.h"
#include "TVector3.h"
#include <cstdlib>
#include <cmath>
#include <random>

#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH2Poly.h"

using namespace Gaugi;
using namespace SG;

MuonCellMerge::MuonCellMerge( std::string name ) :
  IMsgService(name),
  Algorithm()
{
  // --- Propriedades configuráveis ---------------------------------
  declareProperty( "InputCollectionKeys"    , m_collectionKeys={}                   );
  declareProperty( "OutputCellsKey"         , m_cellsKey="MuonCells"                );
  declareProperty( "OutputTruthCellsKey"    , m_truthCellsKey="TruthMuonCells"      );
  declareProperty( "OutputLevel"            , m_outputLevel=1                       );

  // Inicializar gerador aleatório para eficiência
  std::random_device rd;
  m_rng = std::mt19937(rd());
  m_rand_dist = std::uniform_real_distribution<float>(0.0, 1.0);
}

MuonCellMerge::~MuonCellMerge()
{;}

//!=====================================================================

StatusCode MuonCellMerge::initialize()
{
  CHECK_INIT();
  setMsgLevel( m_outputLevel );
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMerge::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMerge::bookHistograms( SG::EventContext &/*ctx*/ ) const
{
  // Nenhum histograma registrado por este algoritmo
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMerge::pre_execute( SG::EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMerge::execute( SG::EventContext &/*ctx*/ , const G4Step * /*step*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMerge::execute( SG::EventContext &ctx , int /*evt*/ ) const
{
  // Em modo standalone, delega para post_execute
  return post_execute(ctx);
}

// ============================================================
// MuonCellMerge::post_execute - MERGE EM UM ÚNICO CONTAINER
// ============================================================

StatusCode MuonCellMerge::post_execute( SG::EventContext &ctx ) const
{
  MSG_INFO( "========================================" );
  MSG_INFO( "Starting muon collection merge algorithm..." );
  MSG_INFO( "========================================" );

  // ============================================
  // CRIAR UM ÚNICO CONTAINER DE SAÍDA PARA RECO CELLS
  // ============================================
  SG::WriteHandle<xAOD::MuonChamberCellContainer> recoContainer( m_cellsKey , ctx );
  recoContainer.record( std::unique_ptr<xAOD::MuonChamberCellContainer>(new xAOD::MuonChamberCellContainer()) );

  // ============================================
  // CRIAR UM ÚNICO CONTAINER DE SAÍDA PARA TRUTH CELLS
  // ============================================
  SG::WriteHandle<xAOD::MuonChamberCellContainer> truthContainer( m_truthCellsKey , ctx );
  truthContainer.record( std::unique_ptr<xAOD::MuonChamberCellContainer>(new xAOD::MuonChamberCellContainer()) );

  int collection_count = 0;
  int descriptor_count = 0;
  int truth_cell_count = 0;
  int reco_cell_count = 0;

  // Mapa para evitar duplicatas
  std::set<unsigned long> processedDescriptors;

  // ============================================
  // LOOP SOBRE TODAS AS COLEÇÕES DE ENTRADA
  // ============================================
  for ( auto key : m_collectionKeys ){

    MSG_DEBUG( "Reading muon chambers from collection: " << key );
    SG::ReadHandle<xAOD::MuonChamberDetDescriptorCollection> collection( key, ctx );

    if( !collection.isValid() ){
      MSG_WARNING( "Cannot read collection with key: " << key );
      continue;
    }

    collection_count++;

    int coll_desc_count = 0;
    for (const auto &pair : **collection.ptr() )
    {
      coll_desc_count++;
    }

    MSG_DEBUG( "  Descriptors in collection: " << coll_desc_count );

    for (const auto &pair : **collection.ptr() )
    {
      descriptor_count++;

      const xAOD::MuonChamberDescriptor* descriptor = pair.second;

      if (!descriptor) {
        MSG_WARNING( "  Null descriptor found!" );
        continue;
      }

      // Evitar duplicatas (mesmo hash em múltiplas coleções)
      if (processedDescriptors.find(descriptor->hash()) != processedDescriptors.end()) {
        MSG_DEBUG( "  Skipping duplicate descriptor: " << descriptor->hash() );
        continue;
      }
      processedDescriptors.insert(descriptor->hash());

      // Calcular energia total depositada e o ToF mais antigo
      float total_edep = 0.0;
      float earliest_tof = 99999.0;

      for (int bcid = descriptor->bcid_start(); bcid <= descriptor->bcid_end(); ++bcid) {
        float edep = descriptor->edep(bcid);
        if (edep > 0) {
          total_edep += edep;
          float tof = descriptor->tof(bcid);
          if (tof < earliest_tof) earliest_tof = tof;
        }
      }

      // ============================================================
      // CRIA CÉLULA DE TRUTH (SEMPRE) - ADICIONA AO ÚNICO CONTAINER
      // ============================================================
      auto truth_cell = new xAOD::MuonChamberCell(
        descriptor->x(), descriptor->y(), descriptor->z(),
        descriptor->dx(), descriptor->dy(), descriptor->dz()
      );

      truth_cell->setAmplitude( total_edep );
      truth_cell->setTime( (earliest_tof < 99999.0) ? earliest_tof : 0.0 );
      truth_cell->setDescriptor( descriptor );

      truthContainer->push_back( truth_cell );
      truth_cell_count++;

      // ============================================================
      // CRIA CÉLULA RECONSTRUÍDA (SÓ SE TIVER ENERGIA)
      // ============================================================
      float reco_energy = descriptor->e();

      if (reco_energy > 1e-6) {

        float reco_time = descriptor->tau();
        float efficiency = (descriptor->efficiency() > 0) ? descriptor->efficiency() : 0.98;

        // Simular perda de eficiência
        bool is_detected = m_rand_dist(m_rng) < efficiency;

        if (is_detected) {
          auto cell = new xAOD::MuonChamberCell(
            descriptor->x(), descriptor->y(), descriptor->z(),
            descriptor->dx(), descriptor->dy(), descriptor->dz()
          );

          cell->setAmplitude( reco_energy );
          cell->setTime( reco_time );
          cell->setDescriptor( descriptor );

          // ADICIONA AO MESMO CONTAINER RECO
          recoContainer->push_back( cell );
          reco_cell_count++;

          if (reco_cell_count <= 20) {  // Limitar output
            MSG_INFO( "    Reco cell #" << reco_cell_count
                      << ": E=" << reco_energy << " MeV"
                      << ", samp=" << descriptor->sampling()
                      << ", det=" << descriptor->chamberType()
                      << ", reg=" << descriptor->region() );
          }
        }
      }
    }
  }

  // ============================================
  // RESUMO FINAL
  // ============================================
  MSG_INFO( "========================================" );
  MSG_INFO( "MUON CELL MERGE SUMMARY" );
  MSG_INFO( "========================================" );
  MSG_INFO( "Collections processed: " << collection_count );
  MSG_INFO( "Total descriptors: " << descriptor_count );
  MSG_INFO( "Truth cells created: " << truth_cell_count );
  MSG_INFO( "Reco cells created: " << reco_cell_count );
  MSG_INFO( "========================================" );
  MSG_INFO( "Final reco container size: " << recoContainer->size() );
  MSG_INFO( "Final truth container size: " << truthContainer->size() );
  MSG_INFO( "========================================" );

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMerge::fillHistograms( SG::EventContext & /*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}
