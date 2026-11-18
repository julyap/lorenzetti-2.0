// ============================================================================
// Arquivo: MuonCellMerge.h
// Autora: Julya Pacheco Ferreira
// Descrição: Declaração da classe MuonCellMerge (implementação em
//            MuonCellMerge.cxx). Define a interface do algoritmo que funde
//            múltiplas coleções de descriptors de células de muon em uma
//            única coleção de células reconstruídas e uma única coleção de
//            células de verdade.
// ============================================================================

#ifndef MuonCellMerge_h
#define MuonCellMerge_h

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/DataHandle.h"
#include <vector>
#include <string>
#include <random>


class MuonCellMerge : public Gaugi::Algorithm
{
  public:
    /** Constructor **/
    MuonCellMerge( std::string name );
    /** Destructor **/
    ~MuonCellMerge();

    // --- Ciclo de vida -----------------------------------------------
    virtual StatusCode initialize() override;
    virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode execute( SG::EventContext &ctx , const G4Step *step ) const override;
    virtual StatusCode execute( SG::EventContext &ctx , int evt ) const override;
    virtual StatusCode post_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode finalize() override;

  private:
    // Lista de chaves das coleções de entrada
    std::vector<std::string> m_collectionKeys;

    // Chave da coleção de saída (reco) e da coleção de verdade
    std::string m_cellsKey;
    std::string m_truthCellsKey;

    // Nível de log
    int m_outputLevel;

    // Gerador aleatório para eficiência (thread-safe)
    mutable std::mt19937 m_rng;
    mutable std::uniform_real_distribution<float> m_rand_dist;
};

#endif
