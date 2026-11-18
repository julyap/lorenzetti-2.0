// ============================================================================
// Arquivo: MuonHitMerge.h
// Autora: Julya Pacheco Ferreira
// Descrição: Declaração da classe MuonHitMerge (implementação em
//            MuonHitMerge.cxx). Define a interface do algoritmo que funde
//            múltiplas coleções de hits de muon em uma única coleção.
// ============================================================================

#ifndef MuonHitMerge_h
#define MuonHitMerge_h

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/DataHandle.h"
#include "G4Step.hh"
#include <vector>
#include <string>


class MuonHitMerge : public Gaugi::Algorithm
{
public:

    /** Constructor **/
    MuonHitMerge( std::string name );

    /** Destructor **/
    ~MuonHitMerge();

    /*! initialize the algorithm **/
    virtual StatusCode initialize() override;

    /*! Book all histograms into the current storegate **/
    virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;

    /*! Execute in step action step from geant core **/
    virtual StatusCode execute( SG::EventContext &ctx , const G4Step *step) const override;

    /*! Execute in ComponentAccumulator **/
    virtual StatusCode execute( SG::EventContext &ctx , int evt ) const override;

    /*! execute before start the step action **/
    virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;

    /*! execute after the step action **/
    virtual StatusCode post_execute( SG::EventContext &ctx ) const override;

    /*! fill histogram in the end **/
    virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;

    /*! finalize the algorithm **/
    virtual StatusCode finalize() override;

private:

    // Lista de chaves das coleções de entrada
    std::vector<std::string> m_collectionKeys;

    // Chave da coleção de saída
    std::string m_hitsKey;

    // Nível de saída
    int m_outputLevel;
};

#endif
