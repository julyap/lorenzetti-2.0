#include "MuonCell/MuonChamberConverter.h"
#include "MuonCell/MuonChamberDescriptorConverter.h"

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclass;

// Struct para MuonChamberCell
#pragma link C++ struct xAOD::MuonChamber_t+;
#pragma link C++ class std::vector< xAOD::MuonChamber_t >+;

// Struct para MuonChamberDescriptor  
#pragma link C++ struct xAOD::MuonChamberDescriptor_t+;
#pragma link C++ class std::vector< xAOD::MuonChamberDescriptor_t >+;

// Classes principais (opcional, se quiser linkar as classes também)
#pragma link C++ class xAOD::MuonChamberCell+;
#pragma link C++ class xAOD::MuonChamberDescriptor+;
#pragma link C++ class std::vector< xAOD::MuonChamberCell* >+;
#pragma link C++ class std::vector< xAOD::MuonChamberDescriptor* >+;
#pragma link C++ class xAOD::MuonChamberDescriptorConverter+;


#endif
