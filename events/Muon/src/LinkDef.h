#include "Muon/MuonConverter.h"

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class xAOD::Muon+;
#pragma link C++ class xAOD::MuonContainer+;
#pragma link C++ class xAOD::MuonConverter+;

// Adicione estas linhas para a serialização
#pragma link C++ class xAOD::Muon_t+;
#pragma link C++ class std::vector<xAOD::Muon_t>+;

#endif
