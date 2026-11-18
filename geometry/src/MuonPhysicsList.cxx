// src/MuonPhysicsList.cxx
#include "MuonPhysicsList.hh"

// Física base do Geant4
#include "G4EmStandardPhysics_option4.hh"
#include "G4HadronPhysicsFTFP_BERT.hh"
#include "G4IonPhysics.hh"
#include "G4IonElasticPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"

// Processos de múon
#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"
#include "G4MuMultipleScattering.hh"

// Partículas
#include "G4MuonPlus.hh"
#include "G4MuonMinus.hh"

// INCLUDES NECESSÁRIOS
#include "G4ProcessManager.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicsListHelper.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include <iostream>

// Classe interna para física de múons
class G4MuonPhysics : public G4VPhysicsConstructor {
public:
    G4MuonPhysics(const G4String& name = "muon") : G4VPhysicsConstructor(name) {}
    virtual ~G4MuonPhysics() {}
    
    virtual void ConstructParticle() {
        G4MuonPlus::MuonPlusDefinition();
        G4MuonMinus::MuonMinusDefinition();
    }
    
    virtual void ConstructProcess() {
        G4cout << "=== Adicionando processos detalhados de múon ===" << G4endl;
        
        G4ParticleDefinition* muonPlus = G4MuonPlus::MuonPlusDefinition();
        G4ParticleDefinition* muonMinus = G4MuonMinus::MuonMinusDefinition();
        
        if (!muonPlus || !muonMinus) {
            G4cerr << "ERRO: Partículas de múon não encontradas!" << G4endl;
            return;
        }
        
        G4ProcessManager* pmanPlus = muonPlus->GetProcessManager();
        G4ProcessManager* pmanMinus = muonMinus->GetProcessManager();
        
        if (!pmanPlus || !pmanMinus) {
            G4cerr << "ERRO: Process managers não encontrados!" << G4endl;
            return;
        }
        
        // Adiciona processos para mu+
        pmanPlus->AddProcess(new G4MuMultipleScattering(), -1, 1, 1);
        pmanPlus->AddProcess(new G4MuIonisation(), -1, 2, 2);
        pmanPlus->AddProcess(new G4MuBremsstrahlung(), -1, 3, 3);
        pmanPlus->AddProcess(new G4MuPairProduction(), -1, 4, 4);
        
        // Adiciona processos para mu-
        pmanMinus->AddProcess(new G4MuMultipleScattering(), -1, 1, 1);
        pmanMinus->AddProcess(new G4MuIonisation(), -1, 2, 2);
        pmanMinus->AddProcess(new G4MuBremsstrahlung(), -1, 3, 3);
        pmanMinus->AddProcess(new G4MuPairProduction(), -1, 4, 4);
        
        G4cout << "Processos de múon adicionados com sucesso!" << G4endl;
    }
};

// Implementação da MuonPhysicsList
MuonPhysicsList::MuonPhysicsList() : G4VModularPhysicsList() {
    G4cout << "MuonPhysicsList: Inicializando lista de física personalizada" << G4endl;
    
    defaultCutValue = 0.7*mm;
    
    // Registra os componentes na ordem correta
    RegisterPhysics(new G4DecayPhysics());
    RegisterPhysics(new G4EmStandardPhysics_option4());
    RegisterPhysics(new G4MuonPhysics());
    RegisterPhysics(new G4HadronPhysicsFTFP_BERT());
    RegisterPhysics(new G4IonElasticPhysics());
    RegisterPhysics(new G4IonPhysics());
    RegisterPhysics(new G4StoppingPhysics());
    RegisterPhysics(new G4RadioactiveDecayPhysics());
}

MuonPhysicsList::~MuonPhysicsList() {}

void MuonPhysicsList::ConstructParticle() {
    G4VModularPhysicsList::ConstructParticle();
}

void MuonPhysicsList::ConstructProcess() {
    G4VModularPhysicsList::ConstructProcess();
    AddTransportation();
}

void MuonPhysicsList::SetCuts() {
    SetCutsWithDefault();
}
