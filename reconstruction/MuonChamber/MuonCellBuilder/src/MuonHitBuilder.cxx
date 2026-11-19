// ============================================================================
// Arquivo: MuonHitBuilder.cxx
// Descrição: Implementação (stub) do algoritmo MuonHitBuilder. No estado
//            atual, apenas registra mensagens de inicialização/finalização
//            e retorna SUCCESS. A lógica de fato de criação/merge de hits
//            é orquestrada pelo equivalente em Python (MuonHitBuilder.py)
//            que instancia os MuonHitMaker/MuonHitMerge apropriados.
// ============================================================================

#include "MuonHitBuilder.h"
#include "GaugiKernel/macros.h"


MuonHitBuilder::MuonHitBuilder(const std::string &name)
    : Gaugi::Algorithm()
{}

//!=====================================================================

StatusCode MuonHitBuilder::initialize()
{
    MSG_INFO("Initializing MuonHitBuilder...");
    return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitBuilder::execute()
{
    // Lógica do MuonHitMaker/MuonHitMerge
    return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonHitBuilder::finalize()
{
    MSG_INFO("Finalizing MuonHitBuilder...");
    return StatusCode::SUCCESS;
}
