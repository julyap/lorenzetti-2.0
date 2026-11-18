// ============================================================================
// Arquivo: MuonMaker.h
// Autora: Julya Pacheco Ferreira
// Descrição: Declaração da classe MuonMaker (implementação em MuonMaker.cxx).
//            Define a interface do algoritmo que reconstrói múons MS-only a
//            partir de clusters, incluindo as funções de avaliação de
//            qualidade (evaluateVeryLoose/Loose/Medium/Tight) e os setters
//            de propriedades usados pela configuração em Python.
// ============================================================================

#ifndef MUONMAKER_H
#define MUONMAKER_H

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/StatusCode.h"
#include <vector>
#include <string>

class TH1F;
class TH2F;

namespace SG {
  class EventContext;
}


class MuonMaker : public Gaugi::Algorithm
{
public:
  MuonMaker( std::string name );
  virtual ~MuonMaker();

  // --- Ciclo de vida -----------------------------------------------
  virtual StatusCode initialize() override;
  virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;
  virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;
  virtual StatusCode execute( SG::EventContext &ctx , const G4Step *step ) const override;
  virtual StatusCode execute( SG::EventContext &ctx , int evt ) const override;
  virtual StatusCode post_execute( SG::EventContext &ctx ) const override;
  virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;
  virtual StatusCode finalize() override;

  // Python property setters
  void setProperty( const std::string& name, const std::string& value );
  void setProperty( const std::string& name, int value );
  void setProperty( const std::string& name, const std::vector<float>& value );

private:
  // Evaluation functions with fracMax
  bool evaluateVeryLoose( float totalEnergy, int nCells, float fracMax ) const;
  bool evaluateLoose( float totalEnergy, int nCells, float chi2, float fracMax ) const;
  bool evaluateMedium( float totalEnergy, int nCells, float chi2, float fracMax ) const;
  bool evaluateTight( float totalEnergy, int nCells, float chi2, float fracMax ) const;

  // --- Propriedades configuráveis ---------------------------------
  std::string m_inputClusterKey;
  std::string m_outputMuonKey;
  float m_minEnergy;
  int m_outputLevel;

  // Cut values: [chi2_cut, min_cells, min_fracMax]
  std::vector<float> m_vlooseCuts;  // Very Loose
  std::vector<float> m_looseCuts;   // Loose
  std::vector<float> m_mediumCuts;  // Medium
  std::vector<float> m_tightCuts;   // Tight
};

#endif
