// ============================================================================
// Arquivo: MuonHitBuilder.h
// Autora: Julya Pacheco Ferreira
// Descrição: Declaração da classe MuonHitBuilder (implementação em
//            MuonHitBuilder.cxx). Define a interface mínima do algoritmo
//            com initialize/execute/finalize.
// ============================================================================

#ifndef MUON_HIT_BUILDER_H
#define MUON_HIT_BUILDER_H

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/StatusCode.h"
#include <string>


class MuonHitBuilder : public Gaugi::Algorithm
{
public:
    MuonHitBuilder(const std::string &name = "MuonHitBuilder");
    virtual ~MuonHitBuilder(){}

    // --- Ciclo de vida -----------------------------------------------
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
};

#endif // MUON_HIT_BUILDER_H
