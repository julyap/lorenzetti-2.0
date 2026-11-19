// ============================================================================
// Arquivo: MuonMaker.cxx
// Descrição: Implementação do algoritmo MuonMaker. Recebe a coleção de
//            clusters de múons (InputMuonClusterKey) e produz a coleção de
//            múons reconstruídos (OutputMuonKey). Aplica cortes de qualidade
//            baseados no ATLAS 2016 (arXiv:1603.05598) para quatro categorias
//            de múons MS-only (sem informação do Inner Detector):
//              - Very Loose (aprox. CT muons em |η|<0.1): aceita 1 camada,
//                sem corte eficaz em chi2, fracMax > 0.5;
//              - Loose (MS track padrão): ≥2 camadas, chi2 < 30,
//                fracMax > 0.6;
//              - Medium (MS-only de boa qualidade): ≥2 camadas,
//                chi2 < 15, fracMax > 0.7;
//              - Tight-like (aproximação MS-only, NÃO é o Tight oficial do
//                ATLAS): ≥3 camadas, chi2 < 8, fracMax > 0.8.
//            Para cada cluster que passa em pelo menos uma categoria, cria um
//            xAOD::Muon com η, φ, pT, E, flags de qualidade e variáveis
//            associadas (fracMax, chi2). Também preenche histogramas de
//            controle (1D e 2D).
// ============================================================================

#include "MuonMaker.h"
#include "GaugiKernel/StoreGate.h"
#include "GaugiKernel/DataHandle.h"
#include "GaugiKernel/MsgStream.h"
#include "GaugiKernel/StatusCode.h"
#include "G4SystemOfUnits.hh"
#include "Muon/MuonContainer.h"
#include "MuonCluster/MuonClusterContainer.h"
#include "Muon/Muon.h"
#include "TH1F.h"
#include "TH2F.h"

using namespace SG;
using namespace Gaugi;


MuonMaker::MuonMaker( std::string name ) :
  Gaugi::Algorithm(),
  m_inputClusterKey("MuonClusters"),
  m_outputMuonKey("Muons"),
  m_minEnergy(0.00001*GeV),  // 10 keV - mínimo para MIPs
  m_outputLevel(1)
{
  // --- Declaração de todas as propriedades configuráveis -------------
  declareProperty( "InputMuonClusterKey", m_inputClusterKey );
  declareProperty( "OutputMuonKey", m_outputMuonKey );
  declareProperty( "VLooseCuts", m_vlooseCuts );
  declareProperty( "LooseCuts", m_looseCuts );
  declareProperty( "MediumCuts", m_mediumCuts );
  declareProperty( "TightCuts", m_tightCuts );
  declareProperty( "OutputLevel", m_outputLevel );
}

//!=====================================================================

MuonMaker::~MuonMaker()
{
}

//!=====================================================================

StatusCode MuonMaker::initialize()
{
  CHECK_INIT();

  // Set default cuts based on ATLAS 2016 for MS-only muons
  // Format: [chi2_cut, min_cells, min_fracMax]
  //
  // VERY LOOSE: based on CT muons in central gap (|η|<0.1)
  //   - Accepts 1 layer (MIP signature)
  //   - No effective chi2 cut
  //   - fracMax > 0.5 ensures MIP-like (dominant cell)
  //
  // LOOSE: based on standard MS track reconstruction
  //   - Requires >=2 layers (Section 3.1: "at least two matching segments")
  //   - chi2 < 30 (reasonable quality)
  //   - fracMax > 0.6 (clear MIP)
  //
  // MEDIUM: based on barrel/endcap MS-only muons
  //   - Requires >=2 layers (same as Loose but tighter chi2)
  //   - chi2 < 15 (good fit quality)
  //   - fracMax > 0.7 (clean MIP)
  //
  // TIGHT-LIKE: approximation of ATLAS Tight for MS-only
  //   - Requires >=3 layers (like ME in forward region 2.5<|η|<2.7)
  //   - chi2 < 8 (same as ATLAS Tight for CB muons)
  //   - fracMax > 0.8 (very clean MIP)
  //   - NOTE: this is a TIGHT-LIKE approximation, NOT the official ATLAS
  //     Tight working point (which requires ID+MS combination)

  // Aplica defaults se os cortes não foram configurados externamente
  if (m_vlooseCuts.empty()) m_vlooseCuts = {999.0, 1.0, 0.5};     // Very Loose
  if (m_looseCuts.empty())  m_looseCuts  = {30.0,  2.0, 0.6};     // Loose
  if (m_mediumCuts.empty()) m_mediumCuts = {15.0,  2.0, 0.7};     // Medium
  if (m_tightCuts.empty())  m_tightCuts  = {8.0,   3.0, 0.8};     // Tight-like

  // Logs de configuração e tabela de cortes
  MSG_INFO("MuonMaker initialized with ATLAS 2016 criteria for MS-only muons");
  MSG_INFO("  InputMuonClusterKey: " << m_inputClusterKey);
  MSG_INFO("  OutputMuonKey: " << m_outputMuonKey);
  MSG_INFO("  MinEnergy: " << m_minEnergy/GeV << " GeV (10 keV for MIPs)");
  MSG_INFO("");
  MSG_INFO("  === CUTS BASED ON ATLAS 2016 ===");
  MSG_INFO("  ┌──────────────┬─────────────┬──────────┬─────────────┐");
  MSG_INFO("  │ Category     │ Min Cells   │ Max Chi2 │ Min FracMax │");
  MSG_INFO("  ├──────────────┼─────────────┼──────────┼─────────────┤");
  MSG_INFO("  │ Very Loose   │ >= " << (int)m_vlooseCuts[1] << "           │ < " << m_vlooseCuts[0] << "    │ >= " << m_vlooseCuts[2] << "       │");
  MSG_INFO("  │ Loose        │ >= " << (int)m_looseCuts[1] << "           │ < " << m_looseCuts[0] << "   │ >= " << m_looseCuts[2] << "       │");
  MSG_INFO("  │ Medium       │ >= " << (int)m_mediumCuts[1] << "           │ < " << m_mediumCuts[0] << "   │ >= " << m_mediumCuts[2] << "       │");
  MSG_INFO("  │ Tight-like   │ >= " << (int)m_tightCuts[1] << "           │ < " << m_tightCuts[0] << "    │ >= " << m_tightCuts[2] << "       │");
  MSG_INFO("  └──────────────┴─────────────┴──────────┴─────────────┘");
  MSG_INFO("");
  MSG_INFO("  Reference: ATLAS 2016, arXiv:1603.05598");
  MSG_INFO("    - Section 3.1: MS track reconstruction (>=2 segments)");
  MSG_INFO("    - Section 5.1: ME muons for |η|>2.5 (>=3 layers)");
  MSG_INFO("    - Section 5.1.2: Tight CB muons have chi2<8");
  MSG_INFO("    - Table 1: Loose efficiency 96.7-98.1%, Tight 89.9-91.8%");

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonMaker::bookHistograms( SG::EventContext &ctx ) const
{
  auto store = ctx.getStoreGateSvc();
  store->mkdir("Expert/MuonMaker");
  store->cd("Expert/MuonMaker");

  // --- Histogramas 1D --------------------------------------------
  store->add(new TH1F("muon_quality", ";Quality (1=Vloose,2=Loose,3=Medium,4=Tight-like);Count", 5, 0, 5));
  store->add(new TH1F("muon_pt", ";p_{T} [GeV];Count", 100, 0, 0.001));
  store->add(new TH1F("muon_eta", ";#eta;Count", 100, -2.5, 2.5));
  store->add(new TH1F("muon_phi", ";#phi;Count", 100, -3.2, 3.2));
  store->add(new TH1F("muon_energy", ";Energy [GeV];Count", 100, 0, 0.0001));
  store->add(new TH1F("muon_ncells", ";Number of cells in cluster;Count", 10, 0, 10));
  store->add(new TH1F("muon_chi2", ";#chi^{2};Count", 100, 0, 50));
  store->add(new TH1F("muon_fracmax", ";FracMax (E_{max}^{2}/E_{total});Count", 100, 0, 1.1));

  // --- Histogramas 2D para validação ------------------------------
  store->add(new TH2F("muon_chi2_vs_ncells", ";nCells;#chi^{2}", 10, 0, 10, 100, 0, 50));
  store->add(new TH2F("muon_fracmax_vs_ncells", ";nCells;FracMax", 10, 0, 10, 100, 0, 1.1));
  store->add(new TH2F("muon_chi2_vs_fracmax", ";FracMax;#chi^{2}", 100, 0, 1.1, 100, 0, 50));

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonMaker::pre_execute( SG::EventContext &ctx ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonMaker::execute( SG::EventContext &ctx , const G4Step *step ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonMaker::execute( SG::EventContext &ctx , int evt ) const
{
  // Em modo standalone, delega para post_execute
  return post_execute(ctx);
}

//!=====================================================================

StatusCode MuonMaker::post_execute( SG::EventContext &ctx ) const
{
  // Cria o container de saída (múons)
  SG::WriteHandle<xAOD::MuonContainer> muons(m_outputMuonKey, ctx);
  muons.record( std::unique_ptr<xAOD::MuonContainer>(new xAOD::MuonContainer()) );

  // Lê o container de clusters de entrada
  SG::ReadHandle<xAOD::MuonClusterContainer> clusters(m_inputClusterKey, ctx);

  if( !clusters.isValid() )
  {
    MSG_FATAL("It's not possible to read the xAOD::MuonClusterContainer from this Context using this key " << m_inputClusterKey );
    return StatusCode::FAILURE;
  }

  MSG_INFO("Processing " << clusters->size() << " muon clusters");

  int nVloose = 0, nLoose = 0, nMedium = 0, nTight = 0;

  // ============================================================
  // LOOP SOBRE OS CLUSTERS
  // ============================================================
  for (const auto& clus : **clusters.ptr()) {
    // Variáveis do cluster
    float totalEnergy = clus->totalEnergy();
    float chi2 = clus->chi2();
    int nCells = clus->size();
    float emaxs2 = clus->emaxs2();
    float fracMax = (totalEnergy > 0) ? emaxs2 / totalEnergy : 0.0;

    // Evaluate categories (mutually exclusive, from tightest to loosest)
    bool isTight   = evaluateTight(totalEnergy, nCells, chi2, fracMax);
    bool isMedium  = evaluateMedium(totalEnergy, nCells, chi2, fracMax);
    bool isLoose   = evaluateLoose(totalEnergy, nCells, chi2, fracMax);
    bool isVloose  = evaluateVeryLoose(totalEnergy, nCells, fracMax);

    // Determina a melhor categoria (a mais restritiva que o cluster passa)
    int quality = -1;
    const char* qualityStr = "";

    if (isTight) {
      quality = 4;
      qualityStr = "TIGHT-LIKE";
      nTight++;
    }
    else if (isMedium) {
      quality = 3;
      qualityStr = "MEDIUM";
      nMedium++;
    }
    else if (isLoose) {
      quality = 2;
      qualityStr = "LOOSE";
      nLoose++;
    }
    else if (isVloose) {
      quality = 1;
      qualityStr = "VLOOSE";
      nVloose++;
    }

    // Cria o múon se passou em pelo menos uma categoria
    if (quality >= 1) {
      xAOD::Muon* muon = new xAOD::Muon();
      muon->setEta(clus->eta());
      muon->setPhi(clus->phi());
      float theta = 2 * atan(exp(-clus->eta()));
      float pt = totalEnergy * sin(theta);
      muon->setPt(pt);
      muon->setE(totalEnergy);
      muon->setQuality(quality);
      muon->setIsVeryLoose(isVloose);
      muon->setIsLoose(isLoose);
      muon->setIsMedium(isMedium);
      muon->setIsTight(isTight);

      muon->setFracMax(fracMax);
      muon->setChi2(chi2);
      muon->setQOverPSignificance(0.0);  // Not available for MS-only
      muon->setRhoPrime(0.0);            // Not available for MS-only
      muon->setMuonCluster(clus);

      muons->push_back(muon);

      MSG_DEBUG("  -> MUON created: quality=" << quality << " (" << qualityStr
                << "), nCells=" << nCells << ", chi2=" << chi2
                << ", fracMax=" << fracMax << ", E=" << totalEnergy/GeV << " GeV");
    } else {
      MSG_DEBUG("  -> REJECTED: nCells=" << nCells << ", chi2=" << chi2
                << ", fracMax=" << fracMax << ", E=" << totalEnergy/GeV << " GeV");
    }
  }

  // Sumário final
  MSG_INFO("Created " << muons->size() << " muons (Vloose=" << nVloose
           << ", Loose=" << nLoose << ", Medium=" << nMedium << ", Tight-like=" << nTight << ")");

  return StatusCode::SUCCESS;
}

// ============================================================
// EVALUATION FUNCTIONS - Based on ATLAS 2016 for MS-only muons
// ============================================================

bool MuonMaker::evaluateVeryLoose(float totalEnergy, int nCells, float fracMax) const
{
  // Based on CT muons in central gap (|η|<0.1) - accepts 1 layer
  // fracMax > 0.5 ensures MIP-like signature (dominant cell)
  return (totalEnergy > m_minEnergy) &&
         (nCells >= (int)m_vlooseCuts[1]) &&
         (fracMax >= m_vlooseCuts[2]);
}

//!=====================================================================

bool MuonMaker::evaluateLoose(float totalEnergy, int nCells, float chi2, float fracMax) const
{
  // Based on standard MS track reconstruction
  // Section 3.1: "at least two matching segments to build a track"
  // chi2 < 30 (reasonable quality), fracMax > 0.6 (clear MIP)
  return (totalEnergy > m_minEnergy) &&
         (nCells >= (int)m_looseCuts[1]) &&
         (chi2 < m_looseCuts[0]) &&
         (fracMax >= m_looseCuts[2]);
}

//!=====================================================================

bool MuonMaker::evaluateMedium(float totalEnergy, int nCells, float chi2, float fracMax) const
{
  // Based on barrel/endcap MS-only muons with good quality
  // Requires >=2 layers, chi2 < 15, clean MIP
  return (totalEnergy > m_minEnergy) &&
         (nCells >= (int)m_mediumCuts[1]) &&
         (chi2 < m_mediumCuts[0]) &&
         (fracMax >= m_mediumCuts[2]);
}

//!=====================================================================

bool MuonMaker::evaluateTight(float totalEnergy, int nCells, float chi2, float fracMax) const
{
  // Tight-like approximation for MS-only (NOT the official ATLAS Tight WP)
  // Requires >=3 layers (like ME in forward region 2.5<|η|<2.7)
  // chi2 < 8 (same as ATLAS Tight for CB muons)
  // Very clean MIP signature
  return (totalEnergy > m_minEnergy) &&
         (nCells >= (int)m_tightCuts[1]) &&
         (chi2 < m_tightCuts[0]) &&
         (fracMax >= m_tightCuts[2]);
}

//!=====================================================================

StatusCode MuonMaker::fillHistograms( SG::EventContext &ctx ) const
{
  auto store = ctx.getStoreGateSvc();
  SG::ReadHandle<xAOD::MuonContainer> muons(m_outputMuonKey, ctx);

  if( !muons.isValid() ){
    MSG_WARNING("Cannot get MuonContainer for histogram filling");
    return StatusCode::SUCCESS;
  }

  store->cd("Expert/MuonMaker");

  // --- Loop sobre os múons para preencher histogramas -------------
  for (const auto& muon : **muons.ptr()) {
    store->hist1("muon_quality")->Fill(muon->quality());
    store->hist1("muon_pt")->Fill(muon->pt()/GeV);
    store->hist1("muon_eta")->Fill(muon->eta());
    store->hist1("muon_phi")->Fill(muon->phi());
    store->hist1("muon_energy")->Fill(muon->e()/GeV);
    store->hist1("muon_fracmax")->Fill(muon->fracMax());

    const auto* cluster = muon->muonCluster();
    if (cluster) {
      store->hist1("muon_ncells")->Fill(cluster->size());
      store->hist1("muon_chi2")->Fill(muon->chi2());

      // 2D histograms for validation
      store->hist2("muon_chi2_vs_ncells")->Fill(cluster->size(), muon->chi2());
      store->hist2("muon_fracmax_vs_ncells")->Fill(cluster->size(), muon->fracMax());
      store->hist2("muon_chi2_vs_fracmax")->Fill(muon->fracMax(), muon->chi2());
    }
  }

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonMaker::finalize()
{
  return StatusCode::SUCCESS;
}

// ============================================================
// SETTERS for Python configuration
// ============================================================

void MuonMaker::setProperty( const std::string& name, const std::string& value )
{
  if (name == "InputMuonClusterKey") {
    m_inputClusterKey = value;
  } else if (name == "OutputMuonKey") {
    m_outputMuonKey = value;
  } else {
    MSG_WARNING("Property " << name << " not recognized for string value");
  }
}

//!=====================================================================

void MuonMaker::setProperty( const std::string& name, int value )
{
  if (name == "OutputLevel") {
    m_outputLevel = value;
  } else {
    MSG_WARNING("Property " << name << " not recognized for int value");
  }
}

//!=====================================================================

void MuonMaker::setProperty( const std::string& name, const std::vector<float>& value )
{
  if (name == "TightCuts") {
    m_tightCuts = value;
  } else if (name == "MediumCuts") {
    m_mediumCuts = value;
  } else if (name == "LooseCuts") {
    m_looseCuts = value;
  } else if (name == "VLooseCuts") {
    m_vlooseCuts = value;
  } else {
    MSG_WARNING("Property " << name << " not recognized for vector<float> value");
  }
}
