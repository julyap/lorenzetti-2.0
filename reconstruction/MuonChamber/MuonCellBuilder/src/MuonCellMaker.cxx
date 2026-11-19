// ============================================================================
// Arquivo: MuonCellMaker.cxx
// Descrição: Implementação do algoritmo MuonCellMaker. Responsável por:
//              - criar a coleção de "descriptors" de células de muon
//                (MuonChamberDetDescriptorCollection) com base em uma grade
//                (η, φ) em torno de um raio R, para uma dada combinação de
//                (Detector, Sampling, Region, Layer);
//              - no post_execute, ler os hits de muon (MuonHitContainer),
//                filtrar por (Detector, Sampling, Region), calcular η/φ
//                reais dos hits e acumular a energia depositada por célula;
//              - acionar o PulseGenerator e as Tools (filtro ótimo etc.)
//                por célula com energia;
//              - criar e preencher a coleção final de células
//                (MuonChamberCellContainer) com a energia reconstruída e a
//                energia de verdade.
// ============================================================================

#include "src/MuonPhiRange.h"
#include "MuonCell/MuonChamberDetDescriptor.h"
#include "MuonCell/MuonChamberDetDescriptorCollection.h"
#include "MuonCell/MuonChamberCell.h"
#include "MuonCell/MuonChamberCellContainer.h"
#include "MuonCell/MuonChamberDescriptorConverter.h"
#include "MuonCell/MuonChamberConverter.h"
#include "MuonHit/MuonHitContainer.h"
#include "MuonHit/MuonHitCollection.h"
#include "EventInfo/EventInfoContainer.h"
#include "G4Kernel/constants.h"
#include "MuonCellMaker.h"

#include "TVector3.h"
#include <cstdlib>
#include "G4SystemOfUnits.hh"
#include "TH1F.h"
#include "TH2F.h"
#include "TH2Poly.h"
#include "TGraph.h"
#include <cmath>
#include <set>

using namespace Gaugi;
using namespace SG;

// ============================================================
// FUNÇÃO AUXILIAR PARA CALCULAR ETA COM SINAL CORRETO
// ============================================================
// Usa eta = asinh(z/rho), garantindo que eta < 0 para z < 0 e
// eta > 0 para z > 0.
float MuonCellMaker::calculateEta(float x, float y, float z) const
{
    float rho = std::sqrt(x*x + y*y);

    if (rho < 1e-6) {
        // Caso especial: muito próximo do eixo Z
        return (z > 0) ? 100.0f : -100.0f;
    }

    // Fórmula correta: eta = asinh(z/rho)
    // Isso dá eta negativo para z < 0 e positivo para z > 0
    float eta = std::asinh(z / rho);

    return eta;
}

// ============================================================
// FUNÇÃO AUXILIAR PARA CALCULAR PHI
// ============================================================
// phi = atan2(y, x), no intervalo [-π, π]
float MuonCellMaker::calculatePhi(float x, float y) const
{
    float phi = std::atan2(y, x);
    return phi;
}

// ============================================================
// FUNÇÃO AUXILIAR PARA CALCULAR THETA A PARTIR DO ETA E Z
// ============================================================
// theta = 2·atan(exp(-eta)), com sinal ajustado pelo sinal de z.
float MuonCellMaker::calculateTheta(float eta, float z) const
{
    float theta = 2.0 * std::atan(std::exp(-eta));
    if (z < 0) theta = -theta;  // Ajuste para theta com sinal
    return theta;
}

//!=====================================================================

MuonCellMaker::MuonCellMaker( std::string name ) :
  IMsgService(name),
  Algorithm(),
  m_toolHandles(),
  m_pulseGenerator(nullptr)
{
  // --- Declaração de todas as propriedades configuráveis -------------
  declareProperty( "InputHitsKey"             , m_hitsKey="MuonHits"              );
  declareProperty( "OutputCollectionKey"      , m_collectionKey="MuonChamberCells");
  declareProperty( "EtaBins"                  , m_etaBins                         );
  declareProperty( "PhiBins"                  , m_phiBins                         );
  declareProperty( "RMin"                     , m_rMin=0.0                        );
  declareProperty( "RMax"                     , m_rMax=0.0                        );
  declareProperty( "R"                        , m_r=5000.0                        );
  declareProperty( "Layer"                    , m_layer=0                         );
  declareProperty( "Sampling"                 , m_sampling=1                      );
  declareProperty( "Detector"                 , m_detector=0                      );
  declareProperty( "Region"                   , m_region=0                        );
  declareProperty( "BunchIdStart"             , m_bcid_start=-20                  );
  declareProperty( "BunchIdEnd"               , m_bcid_end=20                     );
  declareProperty( "BunchDuration"            , m_bc_duration=25                  );
  declareProperty( "OutputLevel"              , m_outputLevel=1                   );
  declareProperty( "DetailedHistograms"       , m_detailedHistograms=false        );
  declareProperty( "HistogramPath"            , m_histPath="/MuonCellMaker"       );
}

//!=====================================================================

// Adiciona uma Tool (ex.: MuonOptimalFilter) à lista de ferramentas
void MuonCellMaker::push_back( Gaugi::AlgTool* tool )
{
  m_toolHandles.push_back(tool);
}

// Define a ferramenta responsável por gerar o pulso das amostras
void MuonCellMaker::setPulseGenerator(Gaugi::AlgTool* tool)
{
  m_pulseGenerator = tool;
}

//!=====================================================================

StatusCode MuonCellMaker::initialize()
{
  CHECK_INIT();

  // Inicializa o gerador de pulso, se existir
  if (m_pulseGenerator && m_pulseGenerator->initialize().isFailure()) {
      MSG_FATAL( "It's not possible to initialize Pulse Generator tool." );
  }

  // Número de bins em eta e phi (nº de bordas - 1)
  m_nEtaBins = m_etaBins.size() - 1;
  m_nPhiBins = m_phiBins.size() - 1;

  setMsgLevel( (MSG::Level)m_outputLevel );

  // Logs informativos da configuração
  MSG_INFO( "MuonCellMaker::initialize() for " << name() );
  MSG_INFO( "  Sampling: " << m_sampling );
  MSG_INFO( "  Detector: " << m_detector );
  MSG_INFO( "  Region: " << m_region );
  MSG_INFO( "  BCID Range: [" << m_bcid_start << ", " << m_bcid_end
            << "] = " << (m_bcid_end - m_bcid_start + 1) << " samples" );
  MSG_INFO( "  Bunch Duration: " << m_bc_duration << " ns" );
  MSG_INFO( "  Eta bins: " << m_nEtaBins << " (range: ["
            << m_etaBins.front() << ", " << m_etaBins.back() << "])" );
  MSG_INFO( "  Phi bins: " << m_nPhiBins << " (range: ["
            << m_phiBins.front() << ", " << m_phiBins.back() << "])" );
  MSG_INFO( "  R: " << m_r / 1000.0 << " m" );

  // Inicializa cada Tool
  for ( auto tool : m_toolHandles )
  {
    MSG_DEBUG(tool->name());
    if (tool->initialize().isFailure() )
    {
      MSG_FATAL( "It's not possible to initialize " << tool->name() << " tool." );
    }
  }

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMaker::finalize()
{
  // Finaliza cada Tool
  for ( auto tool : m_toolHandles )
  {
    if (tool->finalize().isFailure() )
    {
      MSG_ERROR( "It's not possible to finalize " << tool->name() << " tool." );
    }
  }
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMaker::bookHistograms( SG::EventContext &ctx ) const
{
  auto store = ctx.getStoreGateSvc();

  // Cria o diretório de histogramas do algoritmo
  store->mkdir(m_histPath);

  // Histogramas 2D (eta x phi) de energia estimada e energia de verdade
  store->add(new TH2F( "cells_e", "Estimated Muon Chamber Cells Energy; #eta; #phi; Energy [MeV]",
                       m_nEtaBins, m_etaBins.data(), m_nPhiBins, m_phiBins.data() ));
  store->add(new TH2F( "cells_edep", "Truth Muon Chamber Cells Energy; #eta; #phi; Energy [MeV]",
                       m_nEtaBins, m_etaBins.data(), m_nPhiBins, m_phiBins.data() ));
  // Resolução de energia e distribuição de eta
  store->add(new TH1F( "res_cells", "(E_{estimated}-E_{dep}); res_{E} [MeV]; Count",
                       100, -200, 200 ));
  store->add(new TH1F( "eta_dist", "Eta Distribution; #eta; Count",
                       100, -3, 3 ));

  if (m_detailedHistograms){
    int nbunchs = m_bcid_end - m_bcid_start + 1;
    store->add(new TH1F( "timesteps", "Step time per bunch; time[ns]; Count",
                         nbunchs*50, (m_bcid_start - 0.5)*m_bc_duration,
                         (m_bcid_end + 0.5)*m_bc_duration) );
    store->add(new TH1F( "main_event_timesteps", "Step time main event; time[ns]; Count",
                         50, -0.5*m_bc_duration, m_bc_duration*0.5 ));
    store->add(new TH2F( "timesteps_per_energy", "Step time per bunch; time [ns]; Energy [MeV];",
                         nbunchs, (m_bcid_start-0.5)*m_bc_duration,
                         (m_bcid_end+0.5)*m_bc_duration,
                         100, 0, 10) );
    store->add(new TH1F( "muon_hits_per_cell", "Number of muon hits per chamber cell; N hits; Count",
                         50, 0, 50 ));
    store->add(new TH1F( "cluster_size", "Cluster size in muon chambers; Cluster size; Count",
                         20, 0, 20 ));
    store->add(new TH1F( "drift_time", "Drift time in muon chambers; Time [ns]; Count",
                         100, 0, 200 ));
  }

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMaker::pre_execute( EventContext &ctx ) const
{
  MSG_DEBUG( "=== MuonCellMaker::pre_execute ===" );
  MSG_DEBUG( "  Algorithm: " << name() );
  MSG_DEBUG( "  Collection key: " << m_collectionKey );
  MSG_DEBUG( "  BCID Range: [" << m_bcid_start << ", " << m_bcid_end
            << "] = " << (m_bcid_end - m_bcid_start + 1) << " samples" );

  // Cria e registra a coleção de descriptors de células
  SG::WriteHandle<xAOD::MuonChamberDetDescriptorCollection> collection( m_collectionKey, ctx );
  collection.record( std::unique_ptr<xAOD::MuonChamberDetDescriptorCollection>(
    new xAOD::MuonChamberDetDescriptorCollection()) );

  // Tamanho de cada célula em eta e phi
  float deltaEta = std::abs(m_etaBins[1] - m_etaBins[0]);
  float deltaPhi = std::abs(m_phiBins[1] - m_phiBins[0]);

  int cellsCreated = 0;

  // --- Loop sobre a grade (eta, phi) --------------------------------
  for ( unsigned etaBin = 0; etaBin < m_nEtaBins; ++etaBin){
    for ( unsigned phiBin = 0; phiBin < m_nPhiBins; ++phiBin){

      float etaCenter = m_etaBins[etaBin] + deltaEta / 2;
      float phiCenter = m_phiBins[phiBin] + deltaPhi / 2;

      unsigned bin = m_nPhiBins * etaBin + phiBin;

      // Converte (R, eta, phi) → (x, y, z)
      float x = m_r * std::cos(phiCenter);
      float y = m_r * std::sin(phiCenter);
      float z = m_r * std::sinh(etaCenter);

      // Tamanhos físicos da célula
      float deltaX = m_r * deltaPhi;
      float deltaY = m_r * deltaPhi;
      float deltaZ = m_r * std::cosh(etaCenter) * deltaEta;

      // Cria o descriptor
      auto *descriptor = new xAOD::MuonChamberDescriptor(
        x, y, z,
        deltaX, deltaY, deltaZ,
        hash(bin),
        m_detector,
        m_sampling,
        m_layer,
        m_region,
        m_bc_duration,
        m_bcid_start,
        m_bcid_end
      );

      // Propriedades físicas default da célula
      descriptor->setEfficiency(0.98);
      descriptor->setNoiseLevel(0.1);
      descriptor->setTimeResolution(1.0);
      descriptor->setSpatialResolution(0.1);
      descriptor->setGasMixture("Ar/CO2 70/30");
      descriptor->setOperatingVoltage(3000);
      descriptor->setDriftVelocity(0.05);

      // Insere na coleção (fatal se hash duplicado)
      if ( !collection->insert( descriptor->hash(), descriptor ) ){
        MSG_FATAL( "It is not possible to include muon chamber cell hash ("
                  << descriptor->hash() << ") into the collection. Hash already exists.");
      }

      cellsCreated++;
    }
  }

  MSG_DEBUG( "  Created " << cellsCreated << " cells" );
  MSG_DEBUG( "=== MuonCellMaker::pre_execute END ===" );

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMaker::execute( EventContext &/*ctx*/ , const G4Step */*step*/ ) const
{
  // Esta ação não é usada no modo step
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMaker::execute( EventContext &ctx , int /*evt*/ ) const
{
  // Em modo standalone: executa pre_execute + post_execute
  if( pre_execute(ctx).isFailure())
  {
    MSG_FATAL("It's not possible to pre_execute. Abort!");
  }

  if( post_execute(ctx).isFailure())
  {
    MSG_FATAL("It's not possible to post_execute. Abort!");
  }

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCellMaker::post_execute( EventContext &ctx ) const
{
    MSG_DEBUG( "=== MuonCellMaker::post_execute START ===" );
    MSG_DEBUG( "  Algorithm: " << name() );
    MSG_DEBUG( "  Collection key: " << m_collectionKey );
    MSG_DEBUG( "  Input hits key: " << m_hitsKey );

    // Lê a coleção de hits
    SG::ReadHandle<xAOD::MuonHitContainer> hits( m_hitsKey, ctx );
    if( !hits.isValid() ){
      MSG_FATAL( "It's not possible to retrieve the MuonHitContainer using this key: " << m_hitsKey );
      return StatusCode::FAILURE;
    }

    // Lê a coleção de descriptors (criada no pre_execute)
    SG::ReadHandle<xAOD::MuonChamberDetDescriptorCollection> collection( m_collectionKey, ctx );
    if( !collection.isValid() ){
      MSG_FATAL( "It's not possible to retrieve the MuonChamberDetDescriptorCollection using this key: " << m_collectionKey );
      return StatusCode::FAILURE;
    }

    // ============================================================
    // CHAVE ÚNICA DO CONTAINER
    // ============================================================
    std::string uniqueCellKey = m_collectionKey + "_Cells";

    // Cria o container de células de saída
    SG::WriteHandle<xAOD::MuonChamberCellContainer> cellContainer( uniqueCellKey, ctx );
    cellContainer.record(
      std::unique_ptr<xAOD::MuonChamberCellContainer>(
        new xAOD::MuonChamberCellContainer()
      )
    );

    const std::vector<const xAOD::MuonHit*>& hitVector = **hits.ptr();

    MSG_INFO( "=== MUON CELL MAKER FILTER ===" );
    MSG_INFO( "  Sampling: " << m_sampling );
    MSG_INFO( "  Detector: " << m_detector );
    MSG_INFO( "  Region: " << m_region );

    int hitsProcessed = 0;
    int hitsMatched = 0;
    int hitsInCell = 0;
    int hitsOutOfEtaPhi = 0;

    float totalEnergyProcessed = 0.0;

    // Mapas para estatística por célula
    std::map<unsigned long, float> energyPerCell;
    std::map<unsigned long, int> hitsPerCell;

    int debugCount = 0;
    int debugEndcapCount = 0;

    // ============================================================
    // LOOP NOS HITS
    // ============================================================
    for (size_t i = 0; i < hitVector.size(); ++i)
    {
        const auto& hit = hitVector[i];

        hitsProcessed++;

        // ============================================================
        // FILTRO DO MAKER
        // ============================================================
        // Só processa hits (Detector, Sampling, Region)
        if (static_cast<int>(hit->sampling()) != m_sampling) continue;
        if (static_cast<int>(hit->detector()) != m_detector) continue;
        if (static_cast<int>(hit->region())   != m_region)   continue;

        hitsMatched++;

        float x = hit->x();
        float y = hit->y();
        float z = hit->z();

        // ============================================================
        // CÁLCULO CORRETO DE ETA COM SINAL (BASEADO NA POSIÇÃO REAL)
        // ============================================================
        float eta = calculateEta(x, y, z);
        float phi = calculatePhi(x, y);

        // DEBUG para Endcap C (region=2)
        if (m_region == 2 && debugEndcapCount < 20) {
            float rho = std::sqrt(x*x + y*y);
            MSG_INFO( "Endcap C DEBUG: z=" << z << " mm, rho=" << rho
                      << " mm, eta=" << eta << ", phi=" << phi );
            debugEndcapCount++;
        }

        // Encontra os bins eta e phi do hit
        int etaBin = find(m_etaBins, eta);
        int phiBin = find(m_phiBins, phi);

        if (etaBin < 0 || phiBin < 0)
        {
            hitsOutOfEtaPhi++;
            continue;
        }

        hitsInCell++;

        unsigned bin = m_nPhiBins * etaBin + phiBin;

        unsigned long desc_hash = hash(bin);

        xAOD::MuonChamberDescriptor *descriptor = nullptr;

        if (!collection->retrieve(desc_hash, descriptor))
        {
            MSG_WARNING( "Descriptor not found for hash " << desc_hash );
            continue;
        }

        // ============================================================
        // USA A ENERGIA TOTAL DO HIT
        // ============================================================
        float totalHitEnergy = hit->energyDeposit();

        if (totalHitEnergy <= 0) {
            continue;
        }

        totalEnergyProcessed += totalHitEnergy;

        energyPerCell[desc_hash] += totalHitEnergy;

        hitsPerCell[desc_hash]++;

        if (debugCount < 10)
        {
            debugCount++;

            MSG_INFO(
              "Hit #" << debugCount
              << " | E=" << totalHitEnergy
              << " MeV"
              << " | sampling=" << static_cast<int>(hit->sampling())
              << " | detector=" << static_cast<int>(hit->detector())
              << " | region=" << static_cast<int>(hit->region())
              << " | hash=" << desc_hash
              << " | eta=" << eta
              << " | phi=" << phi
            );
        }

        // ============================================================
        // ACUMULA ENERGIA NO DESCRIPTOR
        // ============================================================
        float current = descriptor->edep(0);
        descriptor->edep(0, current + totalHitEnergy);
    }

    // ============================================================
    // RESUMO DOS HITS
    // ============================================================
    MSG_INFO( "Hits processed: " << hitsProcessed );
    MSG_INFO( "Hits matched: " << hitsMatched );
    MSG_INFO( "Hits in eta/phi: " << hitsInCell );
    MSG_INFO( "Hits outside eta/phi: " << hitsOutOfEtaPhi );
    MSG_INFO( "Total accumulated energy: " << totalEnergyProcessed << " MeV" );
    MSG_INFO( "Cells with energy: " << energyPerCell.size() );

    // ============================================================
    // GERAR CÉLULAS
    // ============================================================
    int desc_counter = 0;
    int cellsWithEnergy = 0;
    int cellsCreated = 0;

    for ( const auto& p : **collection.ptr() )
    {
        xAOD::MuonChamberDescriptor *descriptor = p.second;
        desc_counter++;

        // Verifica se há energia em algum BCID
        bool hasEnergy = false;
        float totalEdep = 0.0;

        for (int bcid = descriptor->bcid_start();
             bcid <= descriptor->bcid_end();
             ++bcid)
        {
            float e = descriptor->edep(bcid);
            if (e > 0)
            {
                hasEnergy = true;
                totalEdep += e;
            }
        }

        if (!hasEnergy)
            continue;

        cellsWithEnergy++;

        // ============================================================
        // PULSE GENERATOR
        // ============================================================
        if (m_pulseGenerator)
        {
            if (m_pulseGenerator->execute(ctx, descriptor).isFailure())
            {
                MSG_ERROR( "Pulse generator failed." );
                return StatusCode::FAILURE;
            }
        }

        // ============================================================
        // TOOLS
        // ============================================================
        for ( auto tool : m_toolHandles )
        {
            if (tool->execute(ctx, descriptor).isFailure())
            {
                MSG_ERROR( "Tool execution failed: " << tool->name() );
                return StatusCode::FAILURE;
            }
        }

        // ============================================================
        // CREATE CELL - COM ETA CORRETO
        // ============================================================
        auto *cell = new xAOD::MuonChamberCell();

        cell->setX(descriptor->x());
        cell->setY(descriptor->y());
        cell->setZ(descriptor->z());

        cell->setDx(descriptor->dx());
        cell->setDy(descriptor->dy());
        cell->setDz(descriptor->dz());

        cell->setAmplitude(descriptor->e());
        cell->setTime(descriptor->tau());

        float x = descriptor->x();
        float y = descriptor->y();
        float z = descriptor->z();

        // ============================================================
        // CÁLCULO CORRETO DE ETA E PHI PARA A CÉLULA
        // ============================================================
        float eta = calculateEta(x, y, z);
        float phi = calculatePhi(x, y);

        // Theta pode ser calculado a partir do eta, se necessário
        float theta = calculateTheta(eta, z);

        cell->setTheta(theta);
        cell->setPhi(phi);

        cell->m_eta = eta;        // ← ETA COM SINAL CORRETO!
        cell->m_deltaEta = std::abs(m_etaBins[1] - m_etaBins[0]);
        cell->m_deltaPhi = std::abs(m_phiBins[1] - m_phiBins[0]);

        cell->m_energy = totalEdep;

        cell->setIsMuon(true);
        cell->setDescriptor(descriptor);

        cell->m_layer = m_layer;

        // Usa sampling para o nome da estação
        cell->m_samplingName = getSamplingName(m_sampling, m_region, m_detector);

        cell->m_isTrigger = (m_detector == 1 || m_detector == 2);

        cellContainer->push_back(cell);
        cellsCreated++;

        MSG_DEBUG(
          "Created cell | hash=" << descriptor->hash()
          << " | recoE=" << descriptor->e()
          << " | truthE=" << totalEdep
          << " | eta=" << eta
          << " | phi=" << phi
          << " | sampling=" << m_sampling
        );
    }

    // ============================================================
    // RESUMO FINAL
    // ============================================================
    MSG_INFO( "Total descriptors: " << desc_counter );
    MSG_INFO( "Cells with energy: " << cellsWithEnergy );
    MSG_INFO( "Cells created: " << cellsCreated );
    MSG_INFO( "Container size: " << cellContainer->size() );

    MSG_DEBUG( "=== MuonCellMaker::post_execute END ===" );

    return StatusCode::SUCCESS;
}

//!=====================================================================

std::string MuonCellMaker::getSamplingName(int sampling, int region, int detector) const
{
    std::string name;

    // Nome da camada por (detector, sampling)
    if (detector == 0) {  // MDT
        if (sampling == 1) name = "Inner";
        else if (sampling == 2) name = "Middle";
        else if (sampling == 3) name = "Outer";
        else name = "MDT_Sampling" + std::to_string(sampling);
    }
    else if (detector == 1) {  // RPC
        if (sampling == 4) name = "MiddleTrigger";
        else if (sampling == 5) name = "OuterTrigger";
        else name = "RPC_Sampling" + std::to_string(sampling);
    }
    else if (detector == 2) {  // TGC
        if (sampling == 6) name = "InnerTrigger";
        else if (sampling == 7) name = "OuterTrigger";
        else name = "TGC_Sampling" + std::to_string(sampling);
    }
    else {
        name = "Unknown_" + std::to_string(detector);
    }

    // Sufixo por região
    if (region == 0) {
        name += "_Barrel";
    } else if (region == 1) {
        name += "_EndcapA";
    } else if (region == 2) {
        name += "_EndcapC";
    } else {
        name += "_Region" + std::to_string(region);
    }

    return name;
}

//!=====================================================================

StatusCode MuonCellMaker::fillHistograms( EventContext &ctx ) const
{
  auto store = ctx.getStoreGateSvc();
  SG::ReadHandle<xAOD::MuonChamberDetDescriptorCollection> collection( m_collectionKey, ctx );

  if( !collection.isValid() ){
    MSG_FATAL("It's not possible to retrieve the MuonChamberDetDescriptorCollection using this key: " << m_collectionKey);
  }

  store->cd(m_histPath);

  const xAOD::MuonChamberDescriptor *hotCell = nullptr;
  int totalCells = 0;
  int cellsWithHits = 0;
  float maxEnergy = 0.0;
  float totalEstimatedEnergy = 0.0;
  float totalTruthEnergy = 0.0;

  // --- Loop sobre os descriptors para preencher histogramas ----------
  for ( const auto& p : **collection.ptr() ){
    const auto *cell = p.second;
    totalCells++;

    bool hasEnergy = false;
    float totalEdep = 0.0;
    int totalHits = 0;
    float totalDriftTime = 0.0;
    int desc_start = cell->bcid_start();
    int desc_end = cell->bcid_end();

    for (int bcid = desc_start; bcid <= desc_end; ++bcid) {
      float edep = cell->edep(bcid);
      if (edep > 0) {
        hasEnergy = true;
        totalEdep += edep;
        totalHits += cell->hitCount(bcid);
        totalDriftTime += cell->driftTime(bcid);
      }
    }

    if (hasEnergy) {
      cellsWithHits++;
      totalTruthEnergy += totalEdep;
    }

    float x = cell->x();
    float y = cell->y();
    float z = cell->z();

    // ============================================================
    // CÁLCULO CORRETO DE ETA E PHI PARA HISTOGRAMAS
    // ============================================================
    float eta = calculateEta(x, y, z);
    float phi = calculatePhi(x, y);

    // Preencher histograma de eta
    store->hist1("eta_dist")->Fill(eta);

    if(!hotCell || totalEdep > maxEnergy) {
      hotCell = cell;
      maxEnergy = totalEdep;
    }

    float estimatedEnergy = cell->e();
    totalEstimatedEnergy += estimatedEnergy;
    store->hist1("res_cells")->Fill( estimatedEnergy - totalEdep );

    // Preenche histograma 2D (eta, phi) com energia estimada
    {
      int xbin = store->hist2("cells_e")->GetXaxis()->FindBin(eta);
      int ybin = store->hist2("cells_e")->GetYaxis()->FindBin(phi);
      int bin = store->hist2("cells_e")->GetBin(xbin, ybin, 0);
      float energy = store->hist2("cells_e")->GetBinContent(bin);
      store->hist2("cells_e")->SetBinContent(bin, energy + estimatedEnergy);
    }

    // Preenche histograma 2D (eta, phi) com energia de verdade
    {
      int xbin = store->hist2("cells_edep")->GetXaxis()->FindBin(eta);
      int ybin = store->hist2("cells_edep")->GetYaxis()->FindBin(phi);
      int bin = store->hist2("cells_edep")->GetBin(xbin, ybin, 0);
      float energy = store->hist2("cells_edep")->GetBinContent(bin);
      store->hist2("cells_edep")->SetBinContent(bin, energy + totalEdep);
    }

    // Histogramas detalhados por célula
    if (m_detailedHistograms && hasEnergy) {
      if (totalHits > 0) {
        store->hist1("cluster_size")->Fill(totalHits);
        store->hist1("drift_time")->Fill(totalDriftTime / totalHits);
      }
    }
  }

  // --- Sumário e gráfico do pulso da célula mais energética ----------
  if (m_detailedHistograms) {
    MSG_INFO( "Muon Chamber Statistics for " << name() << ":" );
    MSG_INFO( "  Total cells: " << totalCells );
    MSG_INFO( "  Cells with hits: " << cellsWithHits );
    MSG_INFO( "  Hit efficiency: " << (totalCells > 0 ? 100.0*cellsWithHits/totalCells : 0) << "%" );
    MSG_INFO( "  Total truth energy: " << totalTruthEnergy << " MeV" );
    MSG_INFO( "  Total estimated energy: " << totalEstimatedEnergy << " MeV" );

    if(hotCell) {
      MSG_INFO( "  Hot cell - Hash: " << hotCell->hash()
                << ", Sampling: " << hotCell->sampling()
                << ", Layer: " << hotCell->layer()
                << ", Energy: " << maxEnergy << " MeV" );

      if (hotCell->pulse().size() > 0) {
        auto pulse = hotCell->pulse();
        std::vector<float> samples;
        for (int samp=0; samp<(int)pulse.size(); ++samp)
          samples.push_back(samp);
        TGraph *graph = new TGraph(pulse.size(), samples.data(), pulse.data());
        graph->SetName("muon_chamber_pulse");
        graph->SetTitle("Muon Chamber Pulse Shape");
        graph->GetXaxis()->SetTitle("Time Sample");
        graph->GetYaxis()->SetTitle("Amplitude");
        store->add(graph);
      }
    }
  }

  return StatusCode::SUCCESS;
}

//!=====================================================================

int MuonCellMaker::find( const std::vector<float> &vec, float value) const
{
  if (vec.empty()) return -1;


  if (value == vec.back()) return vec.size() - 2;

  // Encontra o primeiro par (left, right) tal que left <= value < right
  auto binIterator = std::adjacent_find( vec.begin(), vec.end(),
    [=](float left, float right){ return left <= value && value < right; } );

  if ( binIterator == vec.end() ) return -1;
  return binIterator - vec.begin();
}

//!=====================================================================

unsigned long int MuonCellMaker::hash(unsigned bin) const
{
  // Codifica (region, detector, sampling, layer, side, bin) em um único
  // unsigned long para servir de hash de célula.
  int side = 0;
  if (m_rMin > 0) side = 1;
  else if (m_rMax < 0) side = 2;

  unsigned long int hash_value =
    (static_cast<unsigned long int>(m_region % 1000) * 1000000000000UL) +
    (static_cast<unsigned long int>(m_detector % 1000) * 1000000000UL) +
    (static_cast<unsigned long int>(m_sampling % 1000) * 1000000UL) +
    (static_cast<unsigned long int>(m_layer % 1000) * 1000UL) +
    (static_cast<unsigned long int>(side % 1000) * 100000UL) +
    (static_cast<unsigned long int>(bin % 1000000UL));

  return hash_value;
}
