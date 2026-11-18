// MuonPhysicsList.hh
#ifndef MUONPHYSICSLIST_HH
#define MUONPHYSICSLIST_HH

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class MuonPhysicsList : public G4VModularPhysicsList {
public:
    MuonPhysicsList();
    virtual ~MuonPhysicsList();

    virtual void ConstructParticle();
    virtual void ConstructProcess();
    virtual void SetCuts();

private:
    G4VPhysicsConstructor* fEmPhysicsList;
    G4VPhysicsConstructor* fHadPhysicsList;
    G4VPhysicsConstructor* fMuonPhysicsList;  // Para processos específicos de múon
};

#endif
