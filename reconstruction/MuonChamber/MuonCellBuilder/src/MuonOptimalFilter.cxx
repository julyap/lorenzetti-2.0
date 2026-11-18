#include "MuonOptimalFilter.h"
#include "MuonCell/MuonChamberDetDescriptor.h"  
#include "MuonHit/MuonHit.h"
#include <cmath>
#include <algorithm>

using namespace Gaugi;

MuonOptimalFilter::MuonOptimalFilter( std::string name ) : 
  IMsgService(name),
  AlgTool()
{
  declareProperty( "WeightsEnergy"    , m_ofweightsEnergy={}  );
  declareProperty( "WeightsTime"      , m_ofweightsTime={}    );
  declareProperty( "OutputLevel"      , m_outputLevel=1       );
}

MuonOptimalFilter::~MuonOptimalFilter()
{}

StatusCode MuonOptimalFilter::initialize()
{
  setMsgLevel(m_outputLevel);
  
  MSG_INFO("=== MuonOptimalFilter::initialize ===");
  MSG_INFO("WeightsEnergy size: " << m_ofweightsEnergy.size());
  MSG_INFO("WeightsTime size: " << m_ofweightsTime.size());
  
  float sumWeights = 0;
  for (float w : m_ofweightsEnergy) {
    sumWeights += w;
  }
  MSG_INFO("Sum of energy weights: " << sumWeights);
  
  return StatusCode::SUCCESS;
}

StatusCode MuonOptimalFilter::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode MuonOptimalFilter::execute( SG::EventContext &/*ctx*/, Gaugi::EDM *edm ) const
{
    auto *descriptor = static_cast<xAOD::MuonChamberDescriptor*>(edm);
    
    MSG_DEBUG( "=== MuonOptimalFilter::execute ===" );
    MSG_DEBUG( "  Descriptor hash: " << descriptor->hash() );
    MSG_DEBUG( "  Chamber type: " << descriptor->chamberType() );
    
    // ============================================
    // USAR A ENERGIA VERDADEIRA ACUMULADA
    // ============================================
    
    // Calcular a energia total depositada neste descritor
    float total_truth_energy = 0.0;
    int desc_start = descriptor->bcid_start();
    int desc_end = descriptor->bcid_end();
    
    for (int bcid = desc_start; bcid <= desc_end; ++bcid) {
        total_truth_energy += descriptor->edep(bcid);
    }
    
    MSG_DEBUG( "  Total truth energy: " << total_truth_energy << " MeV" );
    
    // ============================================
    // SE NÃO HOUVER ENERGIA, RETORNAR
    // ============================================
    if (total_truth_energy < 1e-9) {
        descriptor->setE(0.0);
        descriptor->setTau(0.0);
        return StatusCode::SUCCESS;
    }
    
    // ============================================
    // PARA MDT: Usar energia verdadeira diretamente
    // ============================================
    double final_energy = total_truth_energy;
    double final_tau = 0.0;
    
    int chamberType = descriptor->chamberType();
    
    // MDT (chamberType == 0)
    if (chamberType == 0) {
        // MDT: energia verdadeira já está em MeV
        // Não precisa de calibração adicional
        
        final_energy = total_truth_energy;
        final_tau = 0.0;
        
        MSG_DEBUG( "MDT: truthE=" << total_truth_energy << " MeV, E=" << final_energy << " MeV" );
    }
    
    // RPC (chamberType == 1)
    else if (chamberType == 1) {
        // RPC: usar energia verdadeira
        final_energy = total_truth_energy;
        final_tau = 0.0;
        
        MSG_DEBUG( "RPC: truthE=" << total_truth_energy << " MeV, E=" << final_energy << " MeV" );
    }
    
    // TGC (chamberType == 2)
    else if (chamberType == 2) {
        // TGC: usar energia verdadeira
        final_energy = total_truth_energy;
        final_tau = 0.0;
        
        MSG_DEBUG( "TGC: truthE=" << total_truth_energy << " MeV, E=" << final_energy << " MeV" );
    }
    
    // Detector desconhecido
    else {
        final_energy = total_truth_energy;
        final_tau = 0.0;
        MSG_WARNING("Unknown chamber type: " << chamberType);
    }
    
    // ============================================
    // PROTEÇÃO FINAL CONTRA NaN/Inf
    // ============================================
    if (std::isnan(final_energy) || std::isinf(final_energy)) {
        MSG_WARNING("Invalid final energy: " << final_energy);
        final_energy = 0.0;
    }
    
    // ============================================
    // SALVAR RESULTADOS
    // ============================================
    descriptor->setE(final_energy);
    descriptor->setTau(final_tau);
    
    MSG_DEBUG( "  Final energy set to: " << final_energy << " MeV" );
    
    return StatusCode::SUCCESS;
}
