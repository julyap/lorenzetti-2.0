#ifndef DetectorConstruction_v1_h
#define DetectorConstruction_v1_h

#include "GaugiKernel/MsgStream.h"
#include "GaugiKernel/Property.h"
#include "G4VUserDetectorConstruction.hh"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "G4Region.hh"
#include "globals.hh"
#include "G4Cache.hh"


//enum Plates{
//    Horizontal       = 0,
//    Vertical         = 1,
//};

class G4VPhysicalVolume;

class G4GlobalMagFieldMessenger;

class DetectorConstruction_v1 : public G4VUserDetectorConstruction, public MsgService, public Gaugi::PropertyService
{
  public:


    struct Volume{
      std::string name;
      int plates;
      std::string absorberMaterial;
      std::string gapMaterial;
      int nofLayers;
      double absoThickness;
      double gapThickness;
      double rMin;
      double rMax;
      double zSize;
      double x;
      double y;
      double z;

      double electronCut;
      double positronCut;
      double gammaCut;
      double photonCut;

      // --- Campos aditivos da camara de muons (defaults preservam os volumes
      //     existentes: ECAL/TILE/EMEC/HEC/DM/Crack) ---
      double muonCut;
      bool   isMuonChamber;
      int    chamberType;          // 0=MDT, 1=RPC, 2=TGC
      std::string gasMixture;
      double operatingVoltage;
      double gasPressure;
    };

    DetectorConstruction_v1(std::string);
    virtual ~DetectorConstruction_v1();
    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();

    // get methods
    const G4VPhysicalVolume* GetAbsorberPV() const;
    const G4VPhysicalVolume* GetGapPV() const;

    // Metodo original do calorimetro: MANTIDO INTACTO, apenas com o
    // parametro opcional muonCut no final (default = 1.0*cm) para nao
    // quebrar as chamadas existentes (ECAL/TILE/EMEC/HEC/DM/Crack).
    void AddVolume(std::string region,
                   int plates,
                   std::string absorberMaterial,
                   std::string gapMaterial,
                   int nofLayers,
                   double absoThickness,
                   double gapThickness,
                   double rMin,
                   double rMax,
                   double zSize,
                   double x,
                   double y,
                   double z,
                   // production cuts
                   double electronCut,
                   double positronCut,
                   double gammaCut,
                   double photonCut,
                   // muon cut (aditivo; default 1 cm)
                   double muonCut = 1.0*cm
                   );

    // --- Camara de muons (aditivo) ---
    // Cria um Volume com isMuonChamber=true. Chamado pelo DetectorConstruction_v1.py
    // para cada detector retornado por getMuonBarrelCfg()/getMuonEndcapCfg().
    void AddMuonChamber(std::string name,
                        int chamberType,
                        double rMin,
                        double rMax,
                        double zSize,
                        double zPos,
                        std::string chamberStructure,
                        std::string gasMixture,
                        double operatingVoltage,
                        double gasPressure,
                        double muonCut
                        );

    // Stub (a SD C++ da camara ainda nao foi integrada neste repositorio):
    // registra a intencao no log. Para gerar hits, a MuonSensitiveDetector
    // deve ser implementada e registrada aqui.
    void SetupMuonSensitiveDetectors();

  private:

    std::vector<Volume> m_volumes;

    // methods
    void DefineMaterials();
    
    G4VPhysicalVolume* DefineVolumes();


    void CreateHorizontalPlates(  G4LogicalVolume *worldLV, 
                                  std::string name,  
                                  G4Material *defaltMaterial,
                                  G4Material *absorberMaterial,
                                  G4Material *gapMaterial,
                                  int nofLayers,
                                  double absoThickness,
                                  double gapThickness,
                                  double calorRmin,
                                  double calorZ,
                                  const G4ThreeVector &center_pos,
                                  G4Region* region);

    void CreateVerticalPlates(  G4LogicalVolume *worldLV, 
                                std::string name,  
                                G4Material *defaultMaterial,
                                G4Material *absorberMaterial,
                                G4Material *gapMaterial,
                                int nofLayers,
                                double absoThickness,
                                double gapThickness,
                                double calorRmin,
                                double calorRmax,
                                double calorZ,
                                const G4ThreeVector &center_pos,
                                G4Region *region);

    // --- Camara de muons (aditivo) ---
    // Constroi a camara (estrutura + gap de gas) no world, para um Volume
    // que tenha isMuonChamber=true. Registra cuts de muon (mu-/mu+) e
    // cria a G4Region com o nome do volume.
    void ConstructMuonChamber( G4LogicalVolume *worldLV,
                               const Volume &volume,
                               G4Material *defaultMaterial,
                               G4Region *region );

    G4Region* GetRegion( std::string name );


    bool m_checkOverlaps; // option to activate checking of volumes overlaps
    bool m_useMagneticField;
    bool m_cutOnPhi;
    int m_outputLevel;

    static G4ThreadLocal G4GlobalMagFieldMessenger*  m_magFieldMessenger;
};




#endif
