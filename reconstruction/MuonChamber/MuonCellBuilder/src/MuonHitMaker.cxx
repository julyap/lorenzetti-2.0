// ============================================================================
// Arquivo: MuonHitMaker.cxx
// Autora: Julya Pacheco Ferreira
// Descrição: Implementação do algoritmo MuonHitMaker. Cria a coleção de
//            hits de muon (MuonHitCollection) numa grade cartesiana X/Y/Z
//            configurada, restrita a uma faixa radial (RMin/RMax) e a uma
//            faixa em Z (ZMin/ZMax) compatível com a região do detector
//            (Barrel, Endcap A, Endcap C). Durante o stepping do Geant4,
//            cada step com deposição de energia é mapeado para o bin
//            correspondente e acumulado no hit (via fill). Também preenche
//            histogramas de controle (XY, Z, distribuição de energia).
// ============================================================================

#include "MuonHit/MuonHit.h"
#include "MuonHit/MuonHitCollection.h"
#include "EventInfo/EventInfoContainer.h"
#include "MuonHitMaker.h"

#include "G4Kernel/CaloPhiRange.h"
#include "G4Kernel/constants.h"
#include "TVector3.h"
#include "G4SystemOfUnits.hh"

#include "TH1F.h"
#include "TH2F.h"
#include "TH2Poly.h"
#include "TGraph.h"
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <ctime>

using namespace Gaugi;
using namespace SG;

MuonHitMaker::MuonHitMaker( std::string name ) :
  IMsgService(name),
  Algorithm()
{
  // --- Propriedades configuráveis ---------------------------------
  declareProperty( "OutputCollectionKey"      , m_collectionKey="MuonHitCollection"   );
  declareProperty( "XBins"                    , m_xBins                                );
  declareProperty( "YBins"                    , m_yBins                                );
  declareProperty( "ZBins"                    , m_zBins                                );
  declareProperty( "RMin"                     , m_rMin                                 );
  declareProperty( "RMax"                     , m_rMax                                 );
  declareProperty( "ZMin"                     , m_zMin                                 );
  declareProperty( "ZMax"                     , m_zMax                                 );
  declareProperty( "Sampling"                 , m_sampling                             );
  declareProperty( "Detector"                 , m_detector                             );
  declareProperty( "Region"                   , m_region                               );
  declareProperty( "BunchIdStart"             , m_bcid_start=-7                        );
  declareProperty( "BunchIdEnd"               , m_bcid_end=8                           );
  declareProperty( "BunchDuration"            , m_bc_duration=25                       );
  declareProperty( "OutputLevel"              , m_outputLevel=1                        );
  declareProperty( "DetailedHistograms"       , m_detailedHistograms=false             );
  declareProperty( "HistogramPath"            , m_histPath="/MuonHitMaker"             );
  declareProperty( "SamplingNoiseStd"         , m_noiseStd=0                           );
  declareProperty( "UseCartesianGrid"         , m_useCartesianGrid=true                );
}

//!=====================================================================

StatusCode MuonHitMaker::initialize()
{
  // Logs de configuração
  MSG_INFO("Initializing MuonHitMaker...");
  MSG_INFO("Detector: " << m_detector << ", Sampling: " << m_sampling << ", Region: " << m_region);
  MSG_INFO("R range: [" << m_rMin << ", " << m_rMax << "] mm");
  MSG_INFO("Z range: [" << m_zMin << ", " << m_zMax << "] mm");
  MSG_INFO("Grid: " << m_xBins.size()-1 << " x-bins, "
           << m_yBins.size()-1 << " y-bins, "
           << m_zBins.size()-1 << " z-bins");

  // Aviso específico para Endcap C (Z negativo)
  if (m_region == 2) {
    MSG_WARNING("*** CONFIGURADO PARA ENDCAP C (Z NEGATIVO) ***");
    MSG_WARNING("*** Z range: [" << m_zMin << ", " << m_zMax << "] mm ***");
  }

  CHECK_INIT();
  setMsgLevel( (MSG::Level)m_outputLevel );

  // Número de bins (nº de bordas - 1)
  m_nXBins = m_xBins.size() - 1;
  m_nYBins = m_yBins.size() - 1;
  m_nZBins = m_zBins.size() - 1;

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMaker::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMaker::bookHistograms( SG::EventContext &ctx ) const
{
  MSG_DEBUG("Booking histograms...");
  auto store = ctx.getStoreGateSvc();

  // Cria o diretório de histogramas
  store->mkdir(m_histPath);

  // Histograma 2D (X,Y) de energia depositada
  if (m_nXBins > 0 && m_nYBins > 0) {
    store->add(new TH2F( "hits_xy_edep", "Hit Energy Deposit - XY; X [mm]; Y [mm]; Energy [MeV]",
                         m_nXBins, m_xBins.data(), m_nYBins, m_yBins.data() ));
  }

  // Histograma 1D de energia vs Z
  if (m_nZBins > 0) {
    store->add(new TH1F( "hits_z_edep", "Hit Energy Deposit vs Z; Z [mm]; Energy [MeV]",
                         m_nZBins, m_zBins.data() ));
  }

  // Distribuição geral de energia dos hits
  store->add(new TH1F( "hit_energy_dist", "Hit Energy Distribution; Energy [MeV]; Counts",
                       100, 0, 10 ));

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMaker::pre_execute( EventContext &ctx ) const
{
  MSG_DEBUG("Pre_execute: Creating hits with Detector=" << m_detector
            << ", Sampling=" << m_sampling << ", Region=" << m_region);

  // Cria e registra a coleção de hits
  SG::WriteHandle<xAOD::MuonHitCollection> collection( m_collectionKey, ctx );
  collection.record( std::unique_ptr<xAOD::MuonHitCollection>(new xAOD::MuonHitCollection()) );

  // Cria um hit para cada célula da grade (grid cartesiano)
  if (m_useCartesianGrid && m_nXBins > 0 && m_nYBins > 0) {

    float deltaX = std::abs(m_xBins[1] - m_xBins[0]);
    float deltaY = std::abs(m_yBins[1] - m_yBins[0]);
    float deltaZ = (m_zBins.size() > 1) ? std::abs(m_zBins[1] - m_zBins[0]) : 1.0;

    unsigned int nZ = std::max(1u, (unsigned int)m_nZBins);

    // Converte os inteiros para os enums do xAOD
    xAOD::MuonSampling sampling_enum = static_cast<xAOD::MuonSampling>(m_sampling);
    xAOD::MuonChamberType detector_enum = static_cast<xAOD::MuonChamberType>(m_detector);
    xAOD::MuonRegion region_enum = static_cast<xAOD::MuonRegion>(m_region);

    // Loop sobre a grade (x, y, z)
    for ( unsigned xBin = 0; xBin < m_nXBins; ++xBin) {
      for ( unsigned yBin = 0; yBin < m_nYBins; ++yBin) {
        for ( unsigned zBin = 0; zBin < nZ; ++zBin) {

          float xCenter = m_xBins[xBin] + deltaX / 2;
          float yCenter = m_yBins[yBin] + deltaY / 2;
          float zCenter = (zBin < m_zBins.size() - 1) ? (m_zBins[zBin] + deltaZ / 2) : 0.0f;

          unsigned bin = zBin + nZ * (yBin + m_nYBins * xBin);

          auto *hit = new xAOD::MuonHit( xCenter, yCenter, zCenter,
                                         deltaX, deltaY,
                                         hash(bin),
                                         sampling_enum,
                                         detector_enum,
                                         region_enum,
                                         m_bc_duration, m_bcid_start, m_bcid_end, 0.0f );

          // Insere no container; fatal se hash já existir
          if( !collection->insert( hit->hash(), hit) )
          {
            MSG_FATAL( "Cannot insert hit hash ("<< hit->hash() << ") into the collection. Hash already exists.");
          }

        }
      }
    }
  }

  MSG_DEBUG("Pre_execute done.");
  return StatusCode::SUCCESS;
}

//!=====================================================================

int MuonHitMaker::find( const std::vector<float> &vec, float value) const
{
  if (vec.size() < 2) return -1;

  // Fora da faixa dos bins (com tolerância)
  if (value < vec.front() - 1e-6) return -1;
  if (value > vec.back() + 1e-6) return -1;

  // Primeiro bin com borda >= value
  auto it = std::lower_bound(vec.begin(), vec.end(), value);

  if (it == vec.end()) return -1;

  int index = std::distance(vec.begin(), it);

  // Caso de borda: value exatamente no último bin → penúltimo
  if (index > 0 && index == (int)vec.size() - 1 && value >= vec.back()) {
    index = vec.size() - 2;
  }

  if (index >= (int)vec.size() - 1) return -1;

  return index;
}

//!=====================================================================

int MuonHitMaker::determineRegionFromZ(float z_mm) const
{
  // Determina a região a partir do Z (mm): Barrel (|z| < 8 m),
  // Endcap A (z > 0) e Endcap C (z < 0).
  if (std::abs(z_mm) < 8000.0) {
    return 0;  // Barrel
  } else if (z_mm > 0) {
    return 1;  // Endcap A
  } else {
    return 2;  // Endcap C
  }
}

//!=====================================================================

StatusCode MuonHitMaker::execute( EventContext &ctx , const G4Step *step ) const
{
  // Lê a coleção de hits
  SG::ReadHandle<xAOD::MuonHitCollection> collection( m_collectionKey, ctx );

  if( !collection.isValid() ){
    MSG_FATAL("Cannot retrieve MuonHitCollection with key: " << m_collectionKey);
  }

  // Posição e deposição do step
  G4ThreeVector pos = step->GetPreStepPoint()->GetPosition();
  G4double edep = step->GetTotalEnergyDeposit();
  G4double time = step->GetPreStepPoint()->GetGlobalTime();

  float x = pos.x() / mm;
  float y = pos.y() / mm;
  float z = pos.z() / mm;
  float radius = std::sqrt(x*x + y*y);

  // ============================================================
  // VERIFICAÇÃO DE ENERGIA
  // ============================================================
  if (edep <= 0)
    return StatusCode::SUCCESS;

  // ============================================================
  // VERIFICAÇÃO DE LIMITES - CORRIGIDO!
  // NÃO usar std::abs(z) para não perder o sinal!
  // ============================================================
  if( !(radius >= m_rMin && radius <= m_rMax) )
    return StatusCode::SUCCESS;

  // Verificação de Z baseada na região (sem std::abs!)
  if (m_region == 2) {
    // Endcap C: aceita Z negativo (ZMin e ZMax são negativos)
    if( !(z >= m_zMin && z <= m_zMax) )
      return StatusCode::SUCCESS;
  } else if (m_region == 1) {
    // Endcap A: aceita Z positivo
    if( !(z >= m_zMin && z <= m_zMax) )
      return StatusCode::SUCCESS;
  } else {
    // BARREL CORRIGIDO: NÃO usar std::abs(z)!
    // Comparar z diretamente com m_zMin e m_zMax
    if( z < m_zMin || z > m_zMax )
      return StatusCode::SUCCESS;
  }

  if (!m_useCartesianGrid) {
    return StatusCode::SUCCESS;
  }

  // ============================================================
  // ENCONTRAR BINS
  // ============================================================
  int xBin = find(m_xBins, x);
  if(xBin < 0)
    return StatusCode::SUCCESS;

  int yBin = find(m_yBins, y);
  if(yBin < 0)
    return StatusCode::SUCCESS;

  int zBin = find(m_zBins, z);
  if(zBin < 0)
    return StatusCode::SUCCESS;

  unsigned int nZ = std::max(1u, (unsigned int)m_nZBins);
  unsigned int bin = zBin + nZ * (yBin + m_nYBins * xBin);
  unsigned long hashBin = hash(bin);

  xAOD::MuonHit *hit = nullptr;

  // Se o hit já existe, usa-o; caso contrário, cria on-the-fly
  if(!collection->retrieve(hashBin, hit)){
    MSG_DEBUG("Hit not found for bin index " << bin << ". Creating on-the-fly.");

    float deltaX = std::abs(m_xBins[1] - m_xBins[0]);
    float deltaY = std::abs(m_yBins[1] - m_yBins[0]);
    float xCenter = m_xBins[xBin] + deltaX / 2;
    float yCenter = m_yBins[yBin] + deltaY / 2;
    float zCenter = (zBin < (int)m_zBins.size() - 1) ?
                    (m_zBins[zBin] + std::abs(m_zBins[1] - m_zBins[0]) / 2) : z;

    xAOD::MuonSampling sampling_enum = static_cast<xAOD::MuonSampling>(m_sampling);
    xAOD::MuonChamberType detector_enum = static_cast<xAOD::MuonChamberType>(m_detector);
    xAOD::MuonRegion region_enum = static_cast<xAOD::MuonRegion>(m_region);

    hit = new xAOD::MuonHit( xCenter, yCenter, zCenter,
                             deltaX, deltaY,
                             hashBin,
                             sampling_enum,
                             detector_enum,
                             region_enum,
                             m_bc_duration, m_bcid_start, m_bcid_end,
                             0.0f );

    SG::WriteHandle<xAOD::MuonHitCollection> writeCollection(m_collectionKey, ctx);
    if( !writeCollection->insert( hit->hash(), hit) ) {
      MSG_FATAL( "Cannot insert hit hash ("<< hit->hash() << ") into the collection.");
    }
  }

  // Acumula o step no hit
  hit->fill( step, m_noiseStd );

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMaker::execute( EventContext &/*ctx*/, int /*evt*/ ) const
{
  // Este algoritmo não funciona em modo standalone
  MSG_ERROR("This method cannot be executed in standalone mode.");
  return StatusCode::FAILURE;
}

//!=====================================================================

StatusCode MuonHitMaker::post_execute( EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitMaker::fillHistograms( EventContext &ctx ) const
{
  auto store = ctx.getStoreGateSvc();
  SG::ReadHandle<xAOD::MuonHitCollection> collection( m_collectionKey, ctx );

  if( !collection.isValid() ){
    MSG_FATAL("Cannot retrieve MuonHitCollection with key: " << m_collectionKey);
  }

  store->cd(m_histPath);

  // --- Loop sobre os hits para preencher histogramas -------------
  for ( const auto& p : **collection.ptr() ){
    const auto *hit = p.second;

    // Acumula energia no histograma (X,Y)
    if (m_nXBins > 0 && m_nYBins > 0) {
      int x = store->hist2("hits_xy_edep")->GetXaxis()->FindBin(hit->x());
      int y = store->hist2("hits_xy_edep")->GetYaxis()->FindBin(hit->y());
      int bin = store->hist2("hits_xy_edep")->GetBin(x,y,0);
      float energy = store->hist2("hits_xy_edep")->GetBinContent(bin);
      store->hist2("hits_xy_edep")->SetBinContent(bin, (energy + hit->energyDeposit()));
    }

    // Energia vs Z
    if (m_nZBins > 0) {
      store->hist1("hits_z_edep")->Fill(hit->z(), hit->energyDeposit());
    }

    // Distribuição de energia
    store->hist1("hit_energy_dist")->Fill(hit->energyDeposit());
  }

  return StatusCode::SUCCESS;
}

//!=====================================================================

unsigned long int MuonHitMaker::hash(unsigned bin) const
{
  // Hash único: (detector, region, sampling, bin)
  return (m_detector * 1000000UL + m_region * 100000UL + m_sampling * 10000UL + bin);
}
