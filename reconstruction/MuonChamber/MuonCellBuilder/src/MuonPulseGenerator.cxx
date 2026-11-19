// ============================================================================
// Arquivo: MuonPulseGenerator.cxx
// Descrição: Implementação do MuonPulseGenerator (AlgTool). Gera a forma
//            de onda amostrada de uma célula de muon (MuonChamberDescriptor)
//            com base:
//              - no template de shaper lido de ShaperFile;
//              - na energia total depositada (soma dos edep por BCID) e no
//                tempo médio de drift ponderado por energia;
//              - em parâmetros de ruído (média/desvio gaussianos), pedestal
//                e deformação do pulso.
//            O pulso resultante é armazenado no descriptor via setPulse.
// ============================================================================

#include "MuonPulseGenerator.h"
#include "MuonCell/MuonChamberDetDescriptor.h"
#include "Randomize.hh"
#include "TRandom.h"

using namespace Gaugi;


MuonPulseGenerator::MuonPulseGenerator( std::string name ) :
  IMsgService(name),
  AlgTool(),
  m_shaperZeroIndex(0),
  m_shaperResolution(0),
  m_rng(0)
{
  // --- Propriedades configuráveis ---------------------------------
  declareProperty( "ShaperFile"       , m_shaperFile=""         );
  declareProperty( "Pedestal"         , m_pedestal = 0          );
  declareProperty( "DeformationMean"  , m_deformationMean=0     );
  declareProperty( "DeformationStd"   , m_deformationStd=0      );
  declareProperty( "SamplingRate"     , m_samplingRate=25       );
  declareProperty( "OutputLevel"      , m_outputLevel=1         );
  declareProperty( "NoiseMean"        , m_noiseMean=0           );
  declareProperty( "NoiseStd"         , m_noiseStd=0            );
  declareProperty( "NSamples"         , m_nsamples=7            );
  declareProperty( "StartSamplingBC"  , m_startSamplingBC=0     );
}

//!=====================================================================

MuonPulseGenerator::~MuonPulseGenerator()
{;}

//!=====================================================================

StatusCode MuonPulseGenerator::initialize()
{
  setMsgLevel( (MSG::Level)m_outputLevel );
  MSG_DEBUG( "Reading shaper values from: " << m_shaperFile << " and " << m_nsamples << " samples.");
  ReadShaper( m_shaperFile );
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonPulseGenerator::finalize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonPulseGenerator::execute( SG::EventContext &/*ctx*/, Gaugi::EDM *edm ) const
{

  // No MuonCellMaker, sabemos que o EDM é sempre um MuonChamberDescriptor
  auto *descriptor = static_cast<xAOD::MuonChamberDescriptor*>(edm);

  if (descriptor->hash() == 0 && descriptor->bcid_start() == 0 && descriptor->bcid_end() == 0) {
    MSG_WARNING("Descriptor seems invalid, skipping pulse generation");
    return StatusCode::SUCCESS;
  }
  // ===========================================================

  MSG_DEBUG( "Generating pulse for descriptor hash: " << descriptor->hash() );

  auto pulse_size = m_nsamples;

  // Create an pulse with zeros with n samples
  std::vector<float> pulse_sum(pulse_size, 0.0);

  // ============= CALCULAR ENERGIA TOTAL DO DESCRIPTOR =============
  float total_energy = 0.0;
  int total_hits = 0;
  float avg_time = 0.0;

  for (int bcid = descriptor->bcid_start(); bcid <= descriptor->bcid_end(); ++bcid) {
    float edep = descriptor->edep(bcid);
    total_energy += edep;

    int hits = descriptor->hitCount(bcid);
    total_hits += hits;

    if (hits > 0 && edep > 0) {
      avg_time += descriptor->driftTime(bcid) * edep;
    }
  }

  if (total_energy > 0) {
    avg_time /= total_energy;
  }
  // ================================================================

  // Se não tem energia, não gera pulso
  if (total_energy <= 0) {
    MSG_DEBUG( "No energy deposited, skipping pulse generation" );
    return StatusCode::SUCCESS;
  }

  // ============= GERAR PULSO BASEADO NA ENERGIA =============
  std::vector<float> pulse;
  GenerateDeterministicPulse( pulse, total_energy, avg_time, 0 );
  // ===========================================================

  // Copiar o pulso gerado
  for ( int samp=0; samp < pulse_size && samp < (int)pulse.size(); ++samp ){
    pulse_sum[samp] = pulse[samp];
  }

  // Add gaussian noise
  AddGaussianNoise(pulse_sum, m_noiseMean, m_noiseStd);

  // ============= SET THE PULSE FOR THE DESCRIPTOR! =============
  descriptor->setPulse( pulse_sum );

  MSG_DEBUG( "Generated pulse for descriptor hash " << descriptor->hash()
            << " with energy " << total_energy << " MeV, pulse size: " << pulse_sum.size());
  // =============================================================

  return StatusCode::SUCCESS;
}

//!=====================================================================

void MuonPulseGenerator::ReadShaper( std::string filepath )
{
  std::ifstream file;
  m_timeSeries.clear();
  m_shaper.clear();
  file.open(filepath);
  if (file) {
     int i=0;
     float a, b;
     while (file >> a >> b)
     {
        m_timeSeries.push_back(a);
        m_shaper.push_back(b);
        if (a == 0.0) m_shaperZeroIndex = i;
        i++;
     }
     MSG_DEBUG( "Read " << m_shaper.size() << " samples from shaper file" );
  } else {
    MSG_FATAL( "Invalid shaper path: " << filepath );
  }
  file.close();
  m_shaperResolution = m_shaper.size() > 2 ? m_timeSeries[1] - m_timeSeries[0] : m_timeSeries[0];
}

//!=====================================================================

void MuonPulseGenerator::AddGaussianNoise( std::vector<float> &pulse, float noiseMean, float noiseStd) const
{
  if (noiseStd <= 0) return;

  for ( auto &value : pulse ) {
    value += m_rng.Gaus( noiseMean, noiseStd );
    if (value < 0) value = 0;  // Não pode ter amplitude negativa
  }
}

//!=====================================================================

void MuonPulseGenerator::GenerateDeterministicPulse( std::vector<float> &pulse,
                                                     float amplitude,
                                                     float phase,
                                                     float lag) const
{
  pulse.resize( m_nsamples, 0.0f );

  // Se não tem template de pulso, não gera nada
  if (m_shaper.empty()) {
    MSG_WARNING("Shaper is empty, cannot generate pulse");
    return;
  }

  float shr    = m_shaperResolution;
  float shzi   = m_shaperZeroIndex;

  for (int i = 0; i < m_nsamples; i++) {
    // random deformation
    float deformation = m_rng.Gaus(m_deformationMean, m_deformationStd);

    // Índice no template do shaper
    int shaperIndex = int(shzi) - int(lag / shr) +
                      (i + m_startSamplingBC) * (m_samplingRate / shr) +
                      round(phase / shr);

    if (shaperIndex < 0 || shaperIndex >= (int)m_shaper.size()) {
      pulse[i] = 0;
      continue;
    }

    pulse[i] = amplitude * m_shaper[shaperIndex] + m_pedestal + deformation;
  }
}
