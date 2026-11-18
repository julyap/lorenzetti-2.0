// ============================================================================
// Arquivo: MuonClusterMaker.cxx
// Autora: Julya Pacheco Ferreira
// Descrição: Implementação do algoritmo MuonClusterMaker. Responsável por
//            agrupar ("clusterizar") células de muon em torno de sementes
//            (seeds) com base em janelas de proximidade:
//              - janelas em eta/phi (EtaWindow, PhiWindow);
//              - janelas espaciais em X/Y/Z (XWindow, YWindow, ZWindow);
//              - janela radial (RWindow) e distância máxima à seed
//                (MaxSeedDistance);
//              - energia mínima da semente (MinCenterEnergy);
//              - número mínimo de células por cluster (MinCellsPerCluster);
//              - centroide ponderado por energia (UseEnergyCentroid).
//            Para cada cluster, calcula variáveis de qualidade (fracMax,
//            chi2, q/p significance, rho'), variáveis de tracking
//            (pT, carga, d0, z0) e cortes de qualidade (VeryLoose, Loose,
//            Medium, Tight). Também preenche histogramas de controle.
// ============================================================================

#include "MuonClusterMaker.h"
#include "GaugiKernel/StoreGate.h"
#include "GaugiKernel/DataHandle.h"
#include "GaugiKernel/MsgStream.h"
#include "GaugiKernel/StatusCode.h"
#include "G4Step.hh"
#include "G4Threading.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include <cmath>
#include <limits>
#include <vector>
#include <algorithm>

using namespace SG;
using namespace Gaugi;


MuonClusterMaker::MuonClusterMaker( std::string name ) :
  Gaugi::Algorithm(),
  m_cellsKey("MuonCells"),
  m_seedsKey("Seeds"),
  m_hitsKey("MuonHits"),
  m_clusterKey("MuonClusters"),
  m_etaWindow(0.3),
  m_phiWindow(0.3),
  m_xWindow(1500.0),
  m_yWindow(1500.0),
  m_zWindow(1500.0),
  m_maxSeedDistance(100.0),
  m_rWindow(500.0),
  m_minCellsPerCluster(1),
  m_useEnergyCentroid(true),
  m_calculateShapes(true),
  m_doForwardMoments(true),
  m_histPath("Expert/MuonClusters"),
  m_minCenterEnergy(0.00001*GeV)

{
  // --- Declaração de todas as propriedades configuráveis -------------
  declareProperty( "InputCellsKey", m_cellsKey );
  declareProperty( "InputSeedsKey", m_seedsKey );
  declareProperty( "InputHitsKey", m_hitsKey );
  declareProperty( "OutputClusterKey", m_clusterKey );
  declareProperty( "EtaWindow", m_etaWindow );
  declareProperty( "PhiWindow", m_phiWindow );
  declareProperty( "XWindow", m_xWindow );
  declareProperty( "YWindow", m_yWindow );
  declareProperty( "ZWindow", m_zWindow );
  declareProperty( "DoForwardMoments", m_doForwardMoments );
  declareProperty( "HistPath", m_histPath );
  declareProperty( "MinCenterEnergy", m_minCenterEnergy );
  declareProperty( "OutputLevel", m_outputLevel=1 );

  declareProperty( "MaxSeedDistance", m_maxSeedDistance );
  declareProperty( "RWindow", m_rWindow );
  declareProperty( "MinCellsPerCluster", m_minCellsPerCluster );
  declareProperty( "UseEnergyCentroid", m_useEnergyCentroid );
  declareProperty( "CalculateShapes", m_calculateShapes );

}

//!=====================================================================

MuonClusterMaker::~MuonClusterMaker()
{
}

//!=====================================================================

StatusCode MuonClusterMaker::initialize()
{
  CHECK_INIT();
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonClusterMaker::bookHistograms( SG::EventContext &ctx ) const
{
  auto store = ctx.getStoreGateSvc();

  // Cria o diretório de histogramas do algoritmo
  store->mkdir( m_histPath );

  // Histogramas de controle de cluster
  store->add( new TH1F("muon_cl_e",     ";Count;E_{cluster} [GeV];",         100, 0.0, 0.001 ) );
  store->add( new TH1F("muon_cl_eta",   ";Count;#eta_{cluster};",             100, -2.5, 2.5 ) );
  store->add( new TH1F("muon_cl_phi",   ";Count;#phi_{cluster};",             100, -3.2, 3.2 ) );
  store->add( new TH1F("muon_cl_size",  ";Count;Number of cells in cluster;", 10, 0, 10 ) );
  store->add( new TH1F("muon_cl_chi2",  ";Count;#chi^{2};",                   100, 0, 50 ) );
  store->add( new TH1F("muon_cl_fracMax", ";Count;Frac Max;",                 100, 0, 1.0 ) );
  store->add( new TH1F("muon_clusters_per_event", ";N clusters;Count",        50, 0, 50 ) );
  store->add( new TH1F("muon_q_over_p_sig", ";Count;q/p significance;",       100, 0, 20 ) );
  store->add( new TH1F("muon_rho_prime", ";Count;#rho';",                     100, 0, 1.0 ) );
  store->add( new TH1F("muon_quality", ";Count;Quality (0=Vloose,1=Loose,2=Medium,3=Tight);", 4, 0, 4 ) );

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonClusterMaker::pre_execute( SG::EventContext &ctx ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonClusterMaker::execute( SG::EventContext &ctx , const G4Step *step) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonClusterMaker::execute( SG::EventContext &ctx , int evt ) const
{
  // Em modo standalone, delega para post_execute
  return post_execute(ctx);
}

// ============================================================
// POST_EXECUTE - Creates clusters and calculates tracking
// ============================================================
StatusCode MuonClusterMaker::post_execute( SG::EventContext &ctx ) const
{
  // Cria o container de saída (clusters)
  SG::WriteHandle<xAOD::MuonClusterContainer> clusters(m_clusterKey, ctx);
  clusters.record( std::unique_ptr<xAOD::MuonClusterContainer>(new xAOD::MuonClusterContainer()) );

  // Lê o container de células de muon
  SG::ReadHandle<xAOD::MuonChamberCellContainer> container(m_cellsKey, ctx);

  if( !container.isValid() )
  {
    MSG_FATAL("It's not possible to read the xAOD::MuonChamberCellContainer from this Context using this key " << m_cellsKey );
    return StatusCode::FAILURE;
  }

  const std::vector<const xAOD::MuonChamberCell*>& cells = **container.ptr();
  size_t nCells = cells.size();

  MSG_INFO( "Starting geometry-based clustering with " << nCells << " cells");

  if(nCells == 0){
    MSG_INFO( "No cells found, created 0 clusters" );
    return StatusCode::SUCCESS;
  }

  // Marca quais células já foram usadas em algum cluster
  std::vector<bool> usedCells(nCells, false);
  int nClustersCreated = 0;

  // ============================================================
  // LOOP SOBRE AS CÉLULAS - cada uma pode ser uma semente
  // ============================================================
  for (size_t i = 0; i < nCells; ++i) {
    const auto* seedCell = cells[i];
    if (usedCells[i]) continue;
    if (seedCell->m_energy < m_minCenterEnergy) continue;

    // Cria o cluster em torno da semente
    xAOD::MuonCluster *clus = new xAOD::MuonCluster(0.0, seedCell->m_eta, seedCell->phi(),
                                                     m_etaWindow/2., m_phiWindow/2.);
    clus->setSeed(seedCell);

    float etot = 0.0;
    int nCellsInCluster = 0;
    float sumWeightedEta = 0.0, sumWeightedPhi = 0.0;
    float sumEnergy = 0.0;
    float maxCellEnergy = 0.0;

    // Marca a semente como usada e acumula sua energia
    usedCells[i] = true;

    float weight = seedCell->m_energy;
    if(weight > 0) {
      sumEnergy += weight;
      sumWeightedEta += weight * seedCell->m_eta;
      sumWeightedPhi += weight * seedCell->phi();
      if(weight > maxCellEnergy) maxCellEnergy = weight;
    }
    etot += seedCell->m_energy;
    nCellsInCluster++;
    clus->push_back(seedCell);

    // --- Loop sobre as demais células (candidatas ao cluster) -----
    for (size_t j = 0; j < nCells; ++j) {
      if (usedCells[j]) continue;
      const auto* cell = cells[j];

      // Distâncias espaciais e angulares
      float dx = std::abs( seedCell->x() - cell->x() );
      float dy = std::abs( seedCell->y() - cell->y() );
      float dz = std::abs( seedCell->z() - cell->z() );
      float deltaR = std::sqrt(dx*dx + dy*dy + dz*dz);
      float deltaEta = std::abs( seedCell->m_eta - cell->m_eta );
      float deltaPhi = std::abs( seedCell->phi() - cell->phi() );
      if (deltaPhi > M_PI) deltaPhi = 2*M_PI - deltaPhi;

      // Critérios de inclusão (janelas + distância radial)
      if (dx < m_xWindow && dy < m_yWindow && dz < m_zWindow &&
          deltaEta < m_etaWindow && deltaPhi < m_phiWindow && deltaR < m_rWindow){

        usedCells[j] = true;
        weight = cell->m_energy;
        if(weight > 0) {
          sumEnergy += weight;
          sumWeightedEta += weight * cell->m_eta;
          sumWeightedPhi += weight * cell->phi();
          if(weight > maxCellEnergy) maxCellEnergy = weight;
        }
        etot += cell->m_energy;
        nCellsInCluster++;
        clus->push_back(cell);
      }
    }

    // --- Validação final do cluster -------------------------------
    if(nCellsInCluster >= m_minCellsPerCluster && etot >= m_minCenterEnergy){

      // Atualiza o centroide ponderado por energia (se configurado)
      if(sumEnergy > 0 && m_useEnergyCentroid){
        float clusterEta = sumWeightedEta / sumEnergy;
        float clusterPhi = sumWeightedPhi / sumEnergy;
        clus->setEmaxs2(maxCellEnergy);
        clus->setEta(clusterEta);
        clus->setPhi(clusterPhi);
      }

      // ============================================================
      // CALCULA CHI2
      // ============================================================
      float fracMax = (etot > 0) ? maxCellEnergy / etot : 0.0;
      float chi2 = calculateChi2(clus, etot);
      float qOverPSig = calculateQOverPSignificance(clus, fracMax);
      float rhoPrime = calculateRhoPrime(clus);

      // ============================================================
      // TRACKING VARIABLES - PT, CHARGE, D0, Z0
      // ============================================================

      // Calculate pt from total energy and eta
      float theta = 2.0f * std::atan(std::exp(-clus->eta()));
      float sinTheta = std::sin(theta);
      float pt = etot * sinTheta;

      // Improve pt estimation using curvature if multiple cells
      if (nCellsInCluster >= 2) {
        float R_sum = 0.0f;
        for (const auto cell : clus->cells()) {
          R_sum += std::sqrt(cell->x()*cell->x() + cell->y()*cell->y());
        }
        float R = R_sum / nCellsInCluster;
        float pt_from_curvature = 0.3f * 2.0f * (R / 1000.0f); // B=2T, R in meters
        if (pt_from_curvature > 0) {
          pt = (pt + pt_from_curvature) * 0.5f;
        }
      }
      if (pt < 0.01f) pt = 0.01f;
      if (pt > 1000.0f) pt = 1000.0f;
      clus->setPt(pt);

      // Calculate total momentum
      float p = pt / sinTheta;
      clus->setP(p);

      // Calculate d0, z0, charge from seed cell
      float d0 = 0.0f;
      float z0 = 0.0f;
      int charge = 1;

      const auto* seed = clus->seed();
      if (seed) {
        // d0 = transverse distance to beam line
        d0 = std::sqrt(seed->x()*seed->x() + seed->y()*seed->y());

        // Add sign based on direction relative to track phi
        float phi_cell = std::atan2(seed->y(), seed->x());
        float delta_phi = std::abs(phi_cell - clus->phi());
        if (delta_phi > M_PI) delta_phi = 2.0f * M_PI - delta_phi;
        if (delta_phi > M_PI/2.0f) d0 = -d0;

        // z0 = z position (simplified)
        z0 = seed->z();

        // Estimate charge from curvature if 3+ cells
        if (nCellsInCluster >= 3) {
          const auto& cells_in_clus = clus->cells();
          TVector3 p1(cells_in_clus[0]->x(), cells_in_clus[0]->y(), cells_in_clus[0]->z());
          TVector3 p2(cells_in_clus[1]->x(), cells_in_clus[1]->y(), cells_in_clus[1]->z());
          TVector3 p3(cells_in_clus[2]->x(), cells_in_clus[2]->y(), cells_in_clus[2]->z());

          TVector3 v12 = p2 - p1;
          TVector3 v23 = p3 - p2;
          TVector3 cross = v12.Cross(v23);
          charge = (cross.z() > 0) ? 1 : -1;
        }
      }
      clus->setD0(d0);
      clus->setZ0(z0);
      clus->setCharge(charge);

      // ============================================================
      // QUALITY CUTS
      // ============================================================

      bool isVeryLoose = (etot > 1e-5 * GeV);

      bool isLoose = (fracMax > 0.1f && chi2 < 15.0f && std::abs(qOverPSig) < 10.0f &&
                std::abs(d0) < 1000.0f * mm && std::abs(z0) < 5000.0f * mm);

      bool isMedium = (fracMax > 0.2f && chi2 < 10.0f && std::abs(qOverPSig) < 7.0f &&
                 nCellsInCluster >= 2 && std::abs(d0) < 500.0f * mm && std::abs(z0) < 3000.0f * mm);

      bool isTight = (fracMax > 0.3f && chi2 < 8.0f && std::abs(qOverPSig) < 5.0f &&
                nCellsInCluster >= 3 && std::abs(d0) < 100.0f * mm && std::abs(z0) < 1000.0f * mm);

      int quality = -1;
      if (isTight) quality = 3;
      else if (isMedium) quality = 2;
      else if (isLoose) quality = 1;
      else if (isVeryLoose) quality = 0;

      clus->setFracMax(fracMax);
      clus->setChi2(chi2);
      clus->setQOverPSignificance(qOverPSig);
      clus->setRhoPrime(rhoPrime);
      clus->setQuality(quality);
      clus->setIsVeryLoose(isVeryLoose);
      clus->setIsLoose(isLoose);
      clus->setIsMedium(isMedium);
      clus->setIsTight(isTight);

      MSG_INFO( "Muon cluster: cells=" << nCellsInCluster
                << ", E=" << etot/GeV << " GeV"
                << ", pt=" << pt/GeV << " GeV"
                << ", charge=" << charge
                << ", d0=" << d0/mm << " mm"
                << ", z0=" << z0/mm << " mm"
                << ", chi2=" << chi2
                << ", quality=" << quality);

      clusters->push_back( clus );
      nClustersCreated++;

    } else {
      // Cluster rejeitado (não atende aos mínimos) → libera memória
      MSG_DEBUG( "Cluster rejected" );
      delete clus;
    }
  }

  MSG_INFO( "Created " << nClustersCreated << " muon clusters" );
  return StatusCode::SUCCESS;
}

// ============================================================
// EVALUATION FUNCTIONS
// ============================================================

// Avaliação de qualidade "Loose"
bool MuonClusterMaker::evaluateLoose(float fracMax, float chi2, float qOverPSig) const
{
  if (fracMax < 0.1) return false;
  if (chi2 > 15.0) return false;
  if (std::abs(qOverPSig) > 10.0) return false;
  return true;
}

// Avaliação de qualidade "Medium"
bool MuonClusterMaker::evaluateMedium(float fracMax, float chi2, float qOverPSig, int nCells) const
{
  if (fracMax < 0.2) return false;
  if (chi2 > 10.0) return false;
  if (std::abs(qOverPSig) >= 7.0) return false;
  if (nCells < 1) return false;
  return true;
}

// Avaliação de qualidade "Tight" (usa rho' e q/p sig combinados)
bool MuonClusterMaker::evaluateTight(float fracMax, float chi2, float qOverPSig, float rhoPrime) const
{
  if (fracMax < 0.3) return false;
  if (chi2 >= 8.0) return false;

  float combinedMetric = std::abs(rhoPrime) + 0.1 * std::abs(qOverPSig);
  if (combinedMetric > 1.5) return false;

  if (std::abs(rhoPrime) > 0.3) return false;
  if (std::abs(qOverPSig) > 5.0) return false;

  return true;
}

// ============================================================
// CALCULA CHI2 CORRETO - Baseado em dispersão espacial
// ============================================================
float MuonClusterMaker::calculateChi2(const xAOD::MuonCluster* clus, float totalEnergy) const
{
  const auto& cells = clus->cells();
  int nCells = cells.size();

  // Clusters de 1 célula têm chi2 = 0 (sem variação espacial)
  if (nCells < 2) return 0.0;
  if (totalEnergy <= 0) return 999.0;

  // Calcula a posição média ponderada por energia (centroide do cluster)
  float meanX = 0.0, meanY = 0.0, meanZ = 0.0;
  for (const auto cell : cells) {
    float w = cell->m_energy / totalEnergy;
    meanX += w * cell->x();
    meanY += w * cell->y();
    meanZ += w * cell->z();
  }

  // Calcula a distância máxima entre células (para normalização)
  float maxDist2 = 0.0;
  for (const auto cell : cells) {
    float dx = cell->x() - meanX;
    float dy = cell->y() - meanY;
    float dz = cell->z() - meanZ;
    float dist2 = dx*dx + dy*dy + dz*dz;
    if (dist2 > maxDist2) maxDist2 = dist2;
  }
  float maxDist = std::sqrt(maxDist2);
  if (maxDist < 1.0) maxDist = 1.0;  // Evita divisão por zero

  // Calcula o chi2 como variância espacial normalizada
  // Quanto menor o chi2, mais alinhadas estão as células
  float chi2 = 0.0;
  for (const auto cell : cells) {
    float dx = cell->x() - meanX;
    float dy = cell->y() - meanY;
    float dz = cell->z() - meanZ;
    float dr = std::sqrt(dx*dx + dy*dy + dz*dz);
    chi2 += (dr * dr) / (maxDist * maxDist);
  }

  // Normaliza pelo número de células
  chi2 = chi2 / nCells;

  // Aplica um fator para escalonar (opcional)
  chi2 = chi2 * 10.0;

  // Limita entre 0 e 50
  chi2 = std::max(0.0f, std::min(50.0f, chi2));

  return chi2;
}

// Cálculo da significância de q/p (proxy)
float MuonClusterMaker::calculateQOverPSignificance(const xAOD::MuonCluster* clus, float fracMax) const
{
  float significance = 10.0 * (1.0 - fracMax);
  return std::max(0.0f, std::min(20.0f, significance));
}

// Cálculo de rho' (variância espacial normalizada)
float MuonClusterMaker::calculateRhoPrime(const xAOD::MuonCluster* clus) const
{
  const auto& cells = clus->cells();
  if (cells.size() < 2) return 0.0;

  float totalEnergy = clus->totalEnergy();
  if (totalEnergy <= 0) return 1.0;

  float meanX = 0.0, meanY = 0.0, meanZ = 0.0;
  for (const auto cell : cells) {
    float w = cell->m_energy / totalEnergy;
    meanX += w * cell->x();
    meanY += w * cell->y();
    meanZ += w * cell->z();
  }

  float variance = 0.0;
  for (const auto cell : cells) {
    float w = cell->m_energy / totalEnergy;
    float dx = cell->x() - meanX;
    float dy = cell->y() - meanY;
    float dz = cell->z() - meanZ;
    variance += w * (dx*dx + dy*dy + dz*dz);
  }

  return std::min(1.0f, variance / 10000.0f);
}

// ============================================================
// FILL HISTOGRAMS
// ============================================================
StatusCode MuonClusterMaker::fillHistograms( SG::EventContext &ctx ) const
{
  MSG_DEBUG( "Fill all histograms" );

  auto store = ctx.getStoreGateSvc();
  SG::ReadHandle<xAOD::MuonClusterContainer> clusters( m_clusterKey, ctx );

  if( !clusters.isValid() ){
    MSG_ERROR("Cannot get MuonClusterContainer");
    return StatusCode::FAILURE;
  }

  store->cd(m_histPath);

  int nClusters = 0;

  // --- Loop sobre os clusters para preencher histogramas ---------
  for( const auto& clus : **clusters.ptr() ){
    nClusters++;

    float totalEnergy = clus->totalEnergy();

    store->hist1("muon_cl_e")->Fill(totalEnergy / GeV);
    store->hist1("muon_cl_eta")->Fill(clus->eta());
    store->hist1("muon_cl_phi")->Fill(clus->phi());
    store->hist1("muon_cl_size")->Fill(clus->size());
    store->hist1("muon_cl_chi2")->Fill(clus->chi2());
    store->hist1("muon_cl_fracMax")->Fill(clus->fracMax());
    store->hist1("muon_q_over_p_sig")->Fill(clus->qOverPSignificance());
    store->hist1("muon_rho_prime")->Fill(clus->rhoPrime());

    if (clus->quality() >= 0 && clus->quality() < 4) {
      store->hist1("muon_quality")->Fill(clus->quality());
    }
  }

  store->hist1("muon_clusters_per_event")->Fill(nClusters);

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonClusterMaker::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

void MuonClusterMaker::fillCluster( SG::EventContext &ctx, xAOD::MuonCluster *clus, std::string key ) const
{
  // Not used
}

//!=====================================================================

// Distância em (eta, phi) com wrap em phi
float MuonClusterMaker::dR( float eta1, float phi1, float eta2, float phi2 ) const
{
  float deta = fabs(eta1-eta2);
  float dphi = fabs(phi1-phi2);
  if (dphi > M_PI) dphi = 2*M_PI - dphi;
  return sqrt( deta*deta + dphi*dphi);
}
