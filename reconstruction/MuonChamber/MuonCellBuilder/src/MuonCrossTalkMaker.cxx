// ============================================================================
// Arquivo: MuonCrossTalkMaker.cxx
// Descrição: Implementação do algoritmo MuonCrossTalkMaker. Aplica o efeito
//            de cross-talk (capacitivo + indutivo) nos pulsos de hits de
//            muon. Para cada hit com energia acima de MinEnergy, considera
//            os hits vizinhos numa janela 3×3 (em coordenadas espaciais) e
//            soma a contribuição de cross-talk (via XTalkTF) ao pulso do
//            hit central. Ao final, insere os hits (modificados ou não)
//            na coleção de saída. Tools acopladas (e.g. filtro ótimo) são
//            executadas após a modificação do pulso.
// ============================================================================

#include "MuonCrossTalkMaker.h"

#include "MuonHit/MuonHit.h"
#include "MuonHit/MuonHitContainer.h"
#include "MuonHit/MuonHitCollection.h"
#include "src/MuonDetectorGeometry.h"
#include "G4Kernel/constants.h"

#include "G4SystemOfUnits.hh"

#include <map>


using namespace Gaugi;


MuonCrossTalkMaker::MuonCrossTalkMaker( std::string name ) :
  IMsgService(name),
  Algorithm()
{
  // --- Propriedades configuráveis ---------------------------------
  declareProperty( "InputCollectionKey"     , m_collectionKey="MuonHits"               ); // input
  declareProperty( "OutputCollectionKey"    , m_xtcollectionKey="XTMuonHits"           ); // output
  declareProperty( "MinEnergy"              , m_minEnergy=1*GeV                     );
  declareProperty( "HistogramPath"          , m_histPath="/MuonCrossTalkMakerSimulator"      );
  declareProperty( "OutputLevel"            , m_outputLevel=1                       );
  declareProperty( "XTAmpCapacitive"        , m_AmpXt_C=4.2                         );
  declareProperty( "XTAmpInductive"         , m_AmpXt_L=2.3                         );
  declareProperty( "XTAmpResistive"         , m_AmpXt_R=1.0                         );
}

//!=====================================================================

StatusCode MuonCrossTalkMaker::initialize()
{
  CHECK_INIT();
  setMsgLevel(m_outputLevel);

  // Inicializa cada Tool
  for ( auto tool : m_toolHandles )
  {
    if (tool->initialize().isFailure() )
    {
      MSG_FATAL( "It's not possible to iniatialize " << tool->name() << " tool." );
    }
  }
  return StatusCode::SUCCESS;
}
//!=====================================================================

void MuonCrossTalkMaker::push_back( Gaugi::AlgTool* tool )
{
  m_toolHandles.push_back(tool);
}

//!=====================================================================

StatusCode MuonCrossTalkMaker::bookHistograms( SG::EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCrossTalkMaker::pre_execute( SG::EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCrossTalkMaker::execute( SG::EventContext &/*ctx*/ , const G4Step * /*step*/ ) const
{
  return StatusCode::SUCCESS;
}


//!=====================================================================

StatusCode MuonCrossTalkMaker::execute( SG::EventContext &ctx , int /*evt*/ ) const
{
  MSG_INFO("Executing MuonCrossTalkMaker module...");

  // Vetores temporários usados para debug 
  std::vector < float > samples_xtalk_ind    ;
  std::vector < float > samples_xtalk_cap    ;
  std::vector < float > samples_signal       ;
  std::vector < float > samples_signal_xtalk ;

  // Lê a coleção de hits de entrada
  SG::ReadHandle<xAOD::MuonHitCollection> collection( m_collectionKey, ctx );

  MSG_INFO( "Creating reco XT muon hits containers with key " << m_xtcollectionKey);
  // Cria a coleção de hits de saída (com cross-talk)
  SG::WriteHandle<xAOD::MuonHitCollection> xtCollection( m_xtcollectionKey , ctx );
  xtCollection.record( std::unique_ptr<xAOD::MuonHitCollection>(new xAOD::MuonHitCollection()) );


  // loop over ordinary muon hit container
  for (const auto &pair : **collection.ptr() )
  {
    auto hit = pair.second;
    xAOD::MuonHit *xthit = hit;  // CORREÇÃO: Removido copy()

    // Step 1: check if we need to apply cx method for current hit. Only for hits higher than
    // min energy. 
    bool bCrossTalkMakerConditions = ( !(xthit->energyDeposit() < m_minEnergy) && !(xthit->pulse().size() == 0) );

    // ------------------------------------------------------------------------------
    // If there IS xtalk conditions, apply XT model to hit 1st neighbors,
    // change the current hit pulse, then add hit to new XT hit container.
    // ------------------------------------------------------------------------------
    if (bCrossTalkMakerConditions)
    {
      // CORREÇÃO: Usando métodos disponíveis
      MSG_DEBUG("Detector: "<< static_cast<int>(xthit->detector())
                << ", sampling: " << static_cast<int>(xthit->sampling())
                << ", hash: " << xthit->hash()
                << ", nsamples: " << xthit->pulse().size()
                << ", truthEne: "<< xthit->energyDeposit()
                << ", x/y/z: "<< xthit->x() << "/"<< xthit->y() << "/" << xthit->z() );

      // Step 2: build a 3x3 window around the central hit.
      //    Since this is a hit candidate, lets take all hits around this hits using a 3x3 window.
      std::vector<const xAOD::MuonHit*> hits_around;

      // loop over ordinary hit container
      for (auto &pairAround : **collection.ptr() ){
        auto neighborHit = pairAround.second;
        if ( neighborHit->pulse().size() == 0) continue; // protection: if there is no pulseShape, skip that hit.
        if ( xthit == neighborHit) continue; // central_hit must not belong to hits_around

        // build a 3x3 window around the central hit
        // For muon chambers, we use spatial coordinates instead of eta/phi
        float diffX = std::abs( xthit->x() - neighborHit->x() );
        float diffY = std::abs( xthit->y() - neighborHit->y() );
        float diffZ = std::abs( xthit->z() - neighborHit->z() );

        // Assuming typical muon chamber strip/pad dimensions
        if( diffX <= 3*10.0 && diffY <= 3*10.0 && diffZ <= 3*10.0 ){
          hits_around.push_back(neighborHit);
        }
      }

      // Step 3: Loop over hits_around to extract xtalk effect from the central_hit surroundings.
      std::vector<float> final_xt_pulse(5);

      for (auto neighbor : hits_around){

        auto pulseHitXT = neighbor->pulse();
        std::vector<float> neighbor_xt_pulse;
        float distorted_sample_ind=0, distorted_sample_cap=0;

        // case 1: diagonal from central hit
        if (neighbor->x() != hit->x() && neighbor->y() != hit->y())
        {
          for (unsigned samp_index=0; samp_index<5; ++samp_index)
          {
            distorted_sample_ind = XTalkTF( pulseHitXT[samp_index],samp_index,true, true);
            distorted_sample_cap = 0; // there is no capacitive cross-talk effect in the hit diagonal
            samples_xtalk_ind.push_back(distorted_sample_ind); // histogram
            samples_xtalk_cap.push_back(distorted_sample_cap);  // histogram
            neighbor_xt_pulse.push_back(distorted_sample_ind + distorted_sample_cap);
          }
        }else {
          // case 2: is inside central cross position
          for (int samp_index=0; samp_index<5; samp_index++)
          {
            distorted_sample_ind = XTalkTF( pulseHitXT[samp_index],samp_index,true, true);
            distorted_sample_cap = XTalkTF( pulseHitXT[samp_index],samp_index,true, false);
            samples_xtalk_ind.push_back(distorted_sample_ind); // histogram
            samples_xtalk_cap.push_back(distorted_sample_cap); // histogram
            neighbor_xt_pulse.push_back(distorted_sample_ind + distorted_sample_cap);
          }
        }

        // sum all xtalk effects around center hit
        for (int i=0; i<5; i++){
          final_xt_pulse[i] += neighbor_xt_pulse[i];
        }

      }// end-for in hits_around


      // Step 4: add total pulse distortion from neighbor hits into the central hit of the 3x3 window.
      auto centralHitPulse = hit->pulse();
      auto pulseBefore      = hit->pulse();
      auto energyBefore     = hit->energyDeposit();

      for (int i=0; i<5; i++)
      {
        samples_signal.push_back(centralHitPulse[i]); // add to fillHistograms
        centralHitPulse[i] = centralHitPulse[i] + final_xt_pulse[i];
        samples_signal_xtalk.push_back(centralHitPulse[i]); // add to fillHistograms
      }

      // Step 5: change pulse value of central hit of the 3x3 window with adjacent xtalk effects.
      hit->setPulse(centralHitPulse);

      // Step 6: Call for Estimation Methods tool
      for ( auto tool : m_toolHandles )
      {
        // digitalization
        if( tool->execute( ctx, hit ).isFailure() ){
          MSG_ERROR( "It's not possible to execute the tool with name " << tool->name() );
          return StatusCode::FAILURE;
        }
      }

      auto pulseAfter   = centralHitPulse;
      auto energyAfter  = hit->energyDeposit();

      // fillHistograms( ctx, samples_xtalk_ind, samples_xtalk_cap, samples_signal, samples_signal_xtalk );
      MSG_DEBUG(" e: "<< energyBefore <<"  Pulse before: " << pulseBefore[0] << "   "<< pulseBefore[1] << "   "<< pulseBefore[2] << "   "<< pulseBefore[3] << "   "<< pulseBefore[4]);
      MSG_DEBUG(" e_xt: "<< energyAfter <<"  Pulse after: " << pulseAfter[0] << "   "<< pulseAfter[1] << "   "<< pulseAfter[2] << "   "<< pulseAfter[3] << "   "<< pulseAfter[4]);

      samples_xtalk_ind.clear();
      samples_xtalk_cap.clear();
      samples_signal.clear();
      samples_signal_xtalk.clear();
    }

    // ------------------------------------------------------------------------------
    // If there is NO xtalk conditions, add the hit normally into new XT Container.
    // -------------------------------------------------------------------------------
    if ( !xtCollection->insert( hit->hash(), hit ) ){
        MSG_FATAL( "It is not possible to include hit hash ("<< hit->hash() << ") into the collection. hash already exist.");
    }

  }

  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCrossTalkMaker::fillHistograms( SG::EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonCrossTalkMaker::finalize()
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

StatusCode MuonCrossTalkMaker::post_execute( SG::EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

// Função de transferência do cross-talk: aplica a amplitude e a forma
// (capacitiva vs indutiva) sobre uma amostra do pulso.
float MuonCrossTalkMaker::XTalkTF(float sample, int samp_index, bool diagonal, bool inductive) const
{
  float BaseAmpXTc = m_AmpXt_C/100*sample ;
  float BaseAmpXTl = m_AmpXt_L/100*sample ;
  float XTcSamples = BaseAmpXTc * XTalk       (25*(samp_index+1) , false );
  float XTlSamples = BaseAmpXTl * XTalk       (25*(samp_index+1) , false );

  if (diagonal && inductive){
    return XTlSamples;
  }
  else{
    if (inductive){
      return XTlSamples;
    }
    else{
      return XTcSamples;
    }
  }
}

//!=====================================================================

// Avalia a função analítica do cross-talk (com parâmetros físicos do
// modelo) em x. Se type==true, retorna a derivada; caso contrário, o
// valor da função.
double MuonCrossTalkMaker::XTalk(double x, bool type) const
{
  TF1* XT_cellTF = new TF1("XT_cellTF","((exp(-x/[0])*x*x)/(2 *[0]*[0]*([0] - [1])) - (exp(-(x/[0]))*x*[1])/([0]*pow([0] - [1],2)) + exp(-(x/[0]))*[1]*[1]/pow([0] - [1],3) + (exp(-(x/[1]))*[1]*[1])/pow(-[0] + [1],3) + (1/(2*[2]*[0] *pow(([0] - [1]),3)))*exp(-x* (1/[0] + 1/[1]))* (-2 *exp(x *(1/[0] + 1/[1]))*[0] *pow(([0] - [1]),3) - 2 *exp(x/[0])*[0]*pow([1],3) + exp(x/[1]) *(x*x *pow(([0] - [1]),2) + 2*x*[0]*([0]*[0] - 3*[0]*[1] + 2*[1]*[1]) + 2*[0]*[0]*([0]*[0] - 3*[0]*[1] + 3*[1]*[1]))) + ((1 - (exp((-x + [2])/[0])*(x - [2])*([0] - 2*[1]))/pow(([0] - [1]),2) - (exp((-x + [2])/[0])*(x - [2])*(x- [2]))/(2*[0]*([0] - [1])) + (exp((-x + [2])/[1])*[1]*[1]*[1])/pow(([0] - [1]),3) - (exp((-x + [2])/[0])*[0]*([0]*[0] - 3*[0]*[1] + 3*[1]*[1]))/pow(([0] - [1]),3))* 0.5*( 1+sign(1, x -[2]) ) )/[2])*[3]*[4]*[3]*[0]*[0]",0., m_tmax2);
  XT_cellTF->SetParameter(0, m_taud);
  XT_cellTF->SetParameter(1, m_taupa);
  XT_cellTF->SetParameter(2, m_td);
  XT_cellTF->SetParameter(3, m_Rf);
  XT_cellTF->SetParameter(4, m_C1);

  double xt_cell = 0 ;

  if (type){
    xt_cell = XT_cellTF->Derivative(x) ;
  }
  else {
    xt_cell = XT_cellTF->Eval(x) ;
  }

  delete XT_cellTF ;

  return xt_cell ;
}

//!=====================================================================

// Função analítica do pulso da célula (mesma forma do XTalk, com
// normalização).
double MuonCrossTalkMaker::CellFunction(double x, bool type) const
{
  TF1* CellM = new TF1("CellM","[5]*((exp(-x/[0])*x*x)/(2 *[0]*[0]*([0] - [1])) - (exp(-(x/[0]))*x*[1])/([0]*pow([0] - [1],2)) + exp(-(x/[0]))*[1]*[1]/pow([0] - [1],3) + (exp(-(x/[1]))*[1]*[1])/pow(-[0] + [1],3) + (1/(2*[2]*[0] *pow(([0] - [1]),3)))*exp(-x* (1/[0] + 1/[1]))* (-2 *exp(x *(1/[0] + 1/[1]))*[0] *pow(([0] - [1]),3) - 2 *exp(x/[0])*[0]*pow([1],3) + exp(x/[1]) *(x*x *pow(([0] - [1]),2) + 2*x*[0]*([0]*[0] - 3*[0]*[1] + 2*[1]*[1]) + 2*[0]*[0]*([0]*[0] - 3*[0]*[1] + 3*[1]*[1]))) + ((1 - (exp((-x + [2])/[0])*(x - [2])*([0] - 2*[1]))/pow(([0] - [1]),2) - (exp((-x + [2])/[0])*(x - [2])*(x- [2]))/(2*[0]*([0] - [1])) + (exp((-x + [2])/[1])*[1]*[1]*[1])/pow(([0] - [1]),3) - (exp((-x + [2])/[0])*[0]*([0]*[0] - 3*[0]*[1] + 3*[1]*[1]))/pow(([0] - [1]),3))* 0.5*( 1+sign(1,x -[2]) ) )/[2])*[3]*[4]*[3]*[0]*[0]",0., m_tmax2);

  CellM->SetParameter(0, m_taud);
  CellM->SetParameter(1, m_taupa);
  CellM->SetParameter(2, m_td);
  CellM->SetParameter(3, m_Rf);
  CellM->SetParameter(4, m_C1);

  double cell = 0 ;

  if (type){
      cell = CellM->Derivative(x) ;
  }
  else {
      cell = CellM->Eval(x) ;
  }

  delete CellM ;

  return cell ;
}
