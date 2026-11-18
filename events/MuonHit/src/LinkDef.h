#include "MuonHit/MuonHitConverter.h"

#ifdef __CINT__

// Diretivas de link para o ROOT CINT (interpretador C++)
#pragma link off all globals;     // Desativa link para variáveis globais
#pragma link off all classes;     // Desativa link para todas as classes
#pragma link off all functions;   // Desativa link para todas as funções
#pragma link C++ nestedclass;     // Permite classes aninhadas no dicionário
#pragma link C++ struct xAOD::MuonHit_t+;  // Adiciona a struct MuonHit_t ao dicionário
#pragma link C++ class std::vector< xAOD::MuonHit_t >+; // Adiciona vector de MuonHit_t

#endif
