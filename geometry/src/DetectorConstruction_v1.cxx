#include "DetectorConstruction_v1.h"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4ProductionCuts.hh"
#include "G4AutoDelete.hh"
#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4UniformMagField.hh"
#include "G4ProductionCuts.hh"
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4RegionStore.hh"
#include <string>
#include <sstream>

namespace{
template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}
}

G4ThreadLocal G4GlobalMagFieldMessenger* DetectorConstruction_v1::m_magFieldMessenger = nullptr;


/**
 * @class DetectorConstruction_v1
 * @brief Defines the full detector geometry for the simulation.
 * 
 * This class implements the G4VUserDetectorConstruction interface to build the 
 * Lorenzetti detector (calorimeters) using Geant4 geometry primitives.
 * It manages materials, logical volumes, and placements for both horizontal 
 * and vertical calorimeter plates.
 * 
 * Properties:
 * - UseMagneticField: Toggle global magnetic field (2 Tesla).
 * - CutOnPhi: Restrict the detector to a phi wedge (used for debugging/visualization).
 */
DetectorConstruction_v1::DetectorConstruction_v1(std::string name)
 : 
  IMsgService(name), 
   G4VUserDetectorConstruction(),
   m_checkOverlaps(true)
{
  declareProperty( "UseMagneticField"           , m_useMagneticField=true     );
  declareProperty( "CutOnPhi"                   , m_cutOnPhi=false            );
  declareProperty( "OutputLevel"                , m_outputLevel=0             ); 
}


DetectorConstruction_v1::~DetectorConstruction_v1()
{;}



//
// Add volume
//
void DetectorConstruction_v1::AddVolume(std::string region,
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
                                        double muonCut
                                        )
{
  Volume v;
  v.name             = region;
  v.plates           = plates;
  v.absorberMaterial = absorberMaterial;
  v.gapMaterial      = gapMaterial;
  v.nofLayers        = nofLayers;
  v.absoThickness    = absoThickness;
  v.gapThickness     = gapThickness;
  v.rMin             = rMin;
  v.rMax             = rMax;
  v.zSize            = zSize;
  v.x                = x;
  v.y                = y;
  v.z                = z;
  v.electronCut      = electronCut;
  v.positronCut      = positronCut;
  v.gammaCut         = gammaCut;
  v.photonCut        = photonCut;
  // defaults aditivos (nao usados pelo ramo de calorimetro)
  v.muonCut          = muonCut;
  v.isMuonChamber    = false;
  v.chamberType      = 0;
  v.gasMixture       = "";
  v.operatingVoltage = 0.0;
  v.gasPressure      = 1.0;

  m_volumes.push_back(v);
}


//
// AddMuonChamber (aditivo): cria um Volume com isMuonChamber=true
//
void DetectorConstruction_v1::AddMuonChamber(std::string name,
                                             int chamberType,
                                             double rMin,
                                             double rMax,
                                             double zSize,
                                             double zPos,
                                             std::string chamberStructure,
                                             std::string gasMixture,
                                             double operatingVoltage,
                                             double gasPressure,
                                             double muonCut)
{
  Volume v;
  v.name             = name;
  v.plates           = 0; // barrel (Horizontal); o ramo de muons usa rMin/rMax/zSize
  v.absorberMaterial = chamberStructure;
  v.gapMaterial      = gasMixture;
  v.nofLayers        = 1;
  v.absoThickness    = 0.5*cm;
  v.gapThickness     = 1.0*cm;
  v.rMin             = rMin;
  v.rMax             = rMax;
  v.zSize            = zSize;
  v.x                = 0;
  v.y                = 0;
  v.z                = zPos;
  v.electronCut      = 1.0*mm;
  v.positronCut      = 1.0*mm;
  v.gammaCut         = 1.0*mm;
  v.photonCut        = 1.0*mm;
  v.muonCut          = muonCut;
  v.isMuonChamber    = true;
  v.chamberType      = chamberType;
  v.gasMixture       = gasMixture;
  v.operatingVoltage = operatingVoltage;
  v.gasPressure      = gasPressure;

  m_volumes.push_back(v);
}



G4VPhysicalVolume* DetectorConstruction_v1::Construct()
{
  // Define materials
  DefineMaterials();
  // Define volumes

  G4Material* defaultMaterial  = G4Material::GetMaterial("Vacuum");

  //
  // World
  //
  G4VSolid* worldS = new G4Tubs( "World",     // its name
                                 0,           // R min
                                 6*m,         // R max
                                 10*m,        // Z max
                                 0*deg,       // phi_min
                                 (!m_cutOnPhi)?(360*deg):(235*deg)      // phi_max
                                 );

  G4LogicalVolume* worldLV = new G4LogicalVolume(
                                 worldS,           // its solid
                                 defaultMaterial,  // its material
                                 "World");         // its name

  G4VPhysicalVolume* worldPV = new G4PVPlacement(
                 0,                // no rotation
                 G4ThreeVector(),  // at (0,0,0)
                 worldLV,          // its logical volume
                 "World",          // its name
                 0,                // its mother  volume
                 false,            // no boolean operation
                 0,                // copy number
                 m_checkOverlaps);  // checking overlaps

  

  for (auto &volume : m_volumes){

    MSG_INFO( "Creating Volume with name " << volume.name );

    // --- Ramo aditivo: camara de muons ---
    // Se o Volume foi criado por AddMuonChamber (isMuonChamber=true),
    // construimos a camara (estrutura + gap de gas) e seguimos.
    if (volume.isMuonChamber) {
      auto region = GetRegion(volume.name);
      ConstructMuonChamber(worldLV, volume, defaultMaterial, region);

      G4ProductionCuts* cuts = new G4ProductionCuts();
      cuts->SetProductionCut(volume.gammaCut,    "gamma");
      cuts->SetProductionCut(volume.electronCut, "e-");
      cuts->SetProductionCut(volume.positronCut, "e+");
      cuts->SetProductionCut(volume.photonCut,   "proton");
      cuts->SetProductionCut(volume.muonCut,     "mu-");
      cuts->SetProductionCut(volume.muonCut,     "mu+");
      region->SetProductionCuts(cuts);
      continue;
    }

    if(volume.plates == 0){ //Plates::Horizontal){

      auto region = GetRegion(volume.name);


      // Create a region
      CreateHorizontalPlates( worldLV,
                    volume.name,
                    defaultMaterial, // default
                    G4Material::GetMaterial(volume.absorberMaterial), // absorber
                    G4Material::GetMaterial(volume.gapMaterial), // gap
                    volume.nofLayers, // ATLAS-like
                    volume.absoThickness, // abso
                    volume.gapThickness, // gap
                    volume.rMin, // start radio,
                    volume.zSize ,// z
                    G4ThreeVector(volume.x,volume.y,volume.z),
                    region );
      G4ProductionCuts* cuts=new G4ProductionCuts();
      cuts->SetProductionCut(volume.gammaCut,"gamma");
      cuts->SetProductionCut(volume.electronCut,"e-");
      cuts->SetProductionCut(volume.positronCut,"e+");
      cuts->SetProductionCut(volume.photonCut,"proton");
      // assign cuts to the region and return succesfully
      region->SetProductionCuts(cuts);
      
    }else if (volume.plates == 1){ //Plates::Vertical){

      auto region = GetRegion(volume.name);
      // Create a region
      CreateVerticalPlates( worldLV,
                    volume.name,
                    defaultMaterial, // default
                    G4Material::GetMaterial(volume.absorberMaterial), // absorber
                    G4Material::GetMaterial(volume.gapMaterial), // gap
                    volume.nofLayers, // ATLAS-like
                    volume.absoThickness, // abso
                    volume.gapThickness, // gap
                    volume.rMin, // start radio,
                    volume.rMax,
                    volume.zSize ,// z
                    G4ThreeVector(volume.x,volume.y,volume.z),
                    region );


      G4ProductionCuts* cuts=new G4ProductionCuts();
      cuts->SetProductionCut(volume.gammaCut,"gamma");
      cuts->SetProductionCut(volume.electronCut,"e-");
      cuts->SetProductionCut(volume.positronCut,"e+");
      cuts->SetProductionCut(volume.photonCut,"proton");
      // assign cuts to the region and return succesfully
      region->SetProductionCuts(cuts);
    
    }else{
      MSG_FATAL("Volume type is not recognize. Abort!");
    }
  }// Loop over volumes

  return worldPV;
}


void DetectorConstruction_v1::DefineMaterials()
{
  // Lead material defined using NIST Manager
  G4NistManager* nistManager = G4NistManager::Instance();
  nistManager->FindOrBuildMaterial("G4_Cu");
  nistManager->FindOrBuildMaterial("G4_Pb");
  nistManager->FindOrBuildMaterial("G4_Fe");
  nistManager->FindOrBuildMaterial("G4_Al");
  nistManager->FindOrBuildMaterial("G4_Si");
  nistManager->FindOrBuildMaterial("G4_CESIUM_IODIDE");
  nistManager->FindOrBuildMaterial("PLASTIC SCINTILLATOR");
  // --- Aditivo: materiais usados pela camara de muons ---
  nistManager->FindOrBuildMaterial("G4_Ar");
  nistManager->FindOrBuildMaterial("G4_CO2");
  nistManager->FindOrBuildMaterial("G4_AIR");
  nistManager->FindOrBuildMaterial("G4_POLYSTYRENE");
  G4Element* elH = new G4Element("Hydrogen", "H", 1., 1.0079 * g/mole);
  G4Element* elC = new G4Element("Carbon", "C", 6., 12.011 * g/mole);
  G4Material* scinti = new G4Material("PLASTIC SCINTILLATOR", 1.032 * g/cm3, 2); // Organic plastic scintillator, polystyrene
  scinti->AddElement(elH, 8.5*perCent);
  scinti->AddElement(elC, 91.5*perCent);

  // Liquid argon material
  G4double a;  // mass of a mole;
  G4double z;  // z=mean number of protons;
  G4double density;
  // The argon by NIST Manager is a gas with a different density
  new G4Material("liquidArgon", z=18., a= 39.95*g/mole, density= 1.390*g/cm3);

  // Vacuum
  new G4Material("Vacuum", z=1., a=1.01*g/mole,density= universe_mean_density,kStateGas, 2.73*kelvin, 3.e-18*pascal);

  // Print materials
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}







/**
 * @brief Constructs the horizontal plates of a calorimeter module (Barrel).
 * 
 * Creates a "sandwich" structure of absorber, gap, and absorber layers arranged 
 * radially (for cylindrical/barrel geometry).
 * 
 * @param worldLV Mother logical volume.
 * @param name Unique name for the volume.
 * @param defaultMaterial Envelope material (usually Vacuum).
 * @param absorberMaterial Absorber material (e.g., Pb, Fe).
 * @param gapMaterial Active material (e.g., LAr, Scintillator).
 * @param nofLayers Number of repeating layers.
 * @param absoThickness Thickness of the absorber plate.
 * @param gapThickness Thickness of the active gap.
 * @param calorRmin Inner radius.
 * @param calorZ Total length in Z.
 * @param center_pos Center position (displacement).
 * @param region G4Region associated with this detector part.
 */
void DetectorConstruction_v1::CreateHorizontalPlates(  G4LogicalVolume *worldLV, 
                                                    std::string name,  
                                                    G4Material *defaultMaterial,
                                                    G4Material *absorberMaterial,
                                                    G4Material *gapMaterial,
                                                    int nofLayers,
                                                    double absoThickness,
                                                    double gapThickness,
                                                    double calorRmin,
                                                    double calorZ,
                                                    const G4ThreeVector &center_pos,
                                                    G4Region *region
                                                    ) 

{
  if ( ! defaultMaterial || ! absorberMaterial || ! gapMaterial ) {
    G4ExceptionDescription msg;
    msg << "Cannot retrieve materials already defined.";
    G4Exception("DetectorConstruction_v1::DefineVolumes()", "MyCode0001", FatalException, msg);
  }


  G4double layerThickness=absoThickness+gapThickness; 

  G4VSolid* calorimeterS = new G4Tubs( name,// its name
                                 calorRmin, // R min 1700mm
                                 calorRmin+ nofLayers*layerThickness, // R max 48cm+1700mm
                                 calorZ/2,    // Z max, +250cm
                                 0*deg,     // phi_min
                                 (!m_cutOnPhi)?(360*deg):(235*deg)    // phi_max
                                 ) ;




  G4LogicalVolume* calorLV = new G4LogicalVolume( calorimeterS,     // its solid
                                                  defaultMaterial,  // its material
                                                  name);   // its name
  new G4PVPlacement(
                 0,                 // no rotation
                 center_pos,        // at (0,0,0)
                 calorLV,           // its logical volume
                 name,              // its name
                 worldLV,           // its mother  volume
                 false,             // no boolean operation
                 0,                 // copy number
                 m_checkOverlaps);  // checking overlaps
  
  region->AddRootLogicalVolume(calorLV);



  for (G4int layer=0; layer < nofLayers; ++layer){

    G4VSolid* layerS = new G4Tubs(name+ "_Layer",// its name
                                 calorRmin + layer*layerThickness,        // R min 1700mm
                                 calorRmin + (layer+1)*layerThickness,   // R max 48cm+1700mm
                                 calorZ/2,            // Z max, +250cm
                                 0*deg,             // phi_min
                                 (!m_cutOnPhi)?(360*deg):(235*deg)            // phi_max
                                 ) ;

    G4LogicalVolume* layerLV = new G4LogicalVolume(
                                                  layerS,           // its solid
                                                  defaultMaterial,  // its material
                                                  name+"_Layer");   // its name


    new G4PVPlacement(
                 0,                 // no rotation
                 G4ThreeVector(0,0,0),
                 //center_pos,        // at (0,0,0)
                 layerLV,           // its logical volume
                 name+"_Layer",     // its name
                 calorLV,           // its mother  volume
                 false,             // no boolean operation
                 0,                 // copy number
                 m_checkOverlaps);  // checking overlaps
    


    G4VSolid* absorverS = new G4Tubs( name+"_Abso",// its name
                                 calorRmin + layer*(absoThickness + gapThickness) ,     // R min 1700mm
                                 calorRmin + layer*(absoThickness + gapThickness) + absoThickness,   // R max 48cm+1700mm
                                 calorZ/2,          // Z max, +250cm
                                 0*deg,                 // phi_min
                                 (!m_cutOnPhi)?(360*deg):(235*deg)             // phi_max
                                 ) ;

    G4LogicalVolume* absorverLV = new G4LogicalVolume(
                                                  absorverS,           // its solid
                                                  absorberMaterial,  // its material
                                                  name+"_Abso");         // its name


    new G4PVPlacement(
                 0,                 // no rotation
                 G4ThreeVector(0,0,0),
                 //center_pos,        // at (0,0,0)
                 absorverLV,        // its logical volume
                 name+"_Abso",      // its name
                 layerLV,           // its mother  volume
                 false,             // no boolean operation
                 0,                 // copy number
                 m_checkOverlaps);  // checking overlaps




    G4VSolid* gapS = new G4Tubs( name+"_Gap",// its name
                                 calorRmin + layer*(absoThickness + gapThickness) + absoThickness,   // R max 48cm+1700mm
                                 calorRmin + (layer+1)*(absoThickness + gapThickness),   // R max 48cm+1700mm
                                 calorZ/2,          // Z max, +250cm
                                 0*deg,           // phi_min
                                 (!m_cutOnPhi)?(360*deg):(235*deg)          // phi_max
        ); 

    G4LogicalVolume* gapLV = new G4LogicalVolume(
                                                  gapS,           // its solid
                                                  gapMaterial,  // its material
                                                  name+"_Gap");         // its name


    new G4PVPlacement(
                 0,                 // no rotation
                 G4ThreeVector(0,0,0),
                 //center_pos,        // at (0,0,0)
                 gapLV,             // its logical volume
                 name+"_Gap",       // its name
                 layerLV,           // its mother  volume
                 false,             // no boolean operation
                 0,                 // copy number
                 m_checkOverlaps);  // checking overlaps


  }// Loop over calorimeter layers
}


void DetectorConstruction_v1::CreateVerticalPlates(  G4LogicalVolume *worldLV, 
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
                                                  G4Region *region
                                                  ) 
{
  if ( ! defaultMaterial || ! absorberMaterial || ! gapMaterial ) {
    G4ExceptionDescription msg;
    msg << "Cannot retrieve materials already defined.";
    G4Exception("DetectorConstruction_v1::DefineVolumes()", "MyCode0001", FatalException, msg);
  }

  const bool evenNofLayers = !(nofLayers % 2);
  const bool invertOrder = center_pos.z() < 0.;

  G4double layerThickness=absoThickness+gapThickness; 

  G4VSolid* calorimeterS = new G4Tubs( name,// its name
                                 calorRmin,
                                 calorRmax,
                                 calorZ/2, // Size in z
                                 0*deg,     // phi_min
                                 (!m_cutOnPhi)?(360*deg):(235*deg)    // phi_max
                                 ) ;

  G4LogicalVolume* calorLV = new G4LogicalVolume( calorimeterS,     // its solid
                                                  defaultMaterial,  // its material
                                                  name);   // its name


  new G4PVPlacement(
                 0,                 // no rotation
                 center_pos,        // at (0,0,0)
                 calorLV,           // its logical volume
                 name,              // its name
                 worldLV,           // its mother  volume
                 false,             // no boolean operation
                 0,                 // copy number
                 m_checkOverlaps);  // checking overlaps
  
  region->AddRootLogicalVolume(calorLV);

  for (G4int layer=-nofLayers/2; layer <= nofLayers/2; ++layer){
    if ( layer == 0 && evenNofLayers){
      continue;
    }

    G4VSolid* layerS = new G4Tubs(name+ "_Layer",// its name
                                 calorRmin,
                                 calorRmax,
                                 layerThickness/2,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg)
                                 );

    G4LogicalVolume* layerLV = new G4LogicalVolume(
                                                  layerS,           // its solid
                                                  defaultMaterial,  // its material
                                                  name+"_Layer");   // its name

    const auto layerLVCenter = G4ThreeVector(0,0,
                   ((invertOrder)?-1.:+1.) * 
                   (sign(-layer)*((evenNofLayers)?0.5:0.) + (layer))
                   *layerThickness // center at with respect to the calorimeter center
                 );

    //G4cout << "Creating layer " << layer << " for: " << layerLVCenter << ". Size: " << layerThickness << std::endl;

    new G4PVPlacement(
                 0,                 // no rotation
                 layerLVCenter,     // center
                 layerLV,           // its logical volume
                 name+"_Layer",     // its name
                 calorLV,           // its mother  volume
                 false,             // no boolean operation
                 0,                 // copy number
                 m_checkOverlaps);  // checking overlaps



    G4VSolid* absorverS = new G4Tubs( name+"_Abso",// its name
                                 calorRmin,     // R min
                                 calorRmax,     // R max
                                 absoThickness/2,  // 
                                 0*deg,         // phi_min
                                 (!m_cutOnPhi)?(360*deg):(235*deg)        // phi_max
                                 ) ;

    G4LogicalVolume* absorverLV = new G4LogicalVolume(
                                                  absorverS,           // its solid
                                                  absorberMaterial,  // its material
                                                  name+"_Abso");         // its name


    new G4PVPlacement(
                 0,                 // no rotation
                 G4ThreeVector(0,0,
                   ((invertOrder)?-1.:+1) * 
                   layerThickness/2
                   + ((invertOrder)?+1.:-1)/2 * 
                    absoThickness
                 ),
                 absorverLV,        // its logical volume
                 name+"_Abso",      // its name
                 layerLV,           // its mother  volume
                 false,             // no boolean operation
                 0,                 // copy number
                 m_checkOverlaps);  // checking overlaps




    G4VSolid* gapS = new G4Tubs( name+"_Gap",// its name
                                 calorRmin,   // R max
                                 calorRmax,   // R max
                                 gapThickness/2,   // Z max,
                                 0*deg,           // phi_min
                                 (!m_cutOnPhi)?(360*deg):(235*deg)          // phi_max
        ); 

    G4LogicalVolume* gapLV = new G4LogicalVolume(
                                                  gapS,           // its solid
                                                  gapMaterial,  // its material
                                                  name+"_Gap");         // its name


    new G4PVPlacement(
                 0,                 // no rotation
                 G4ThreeVector(0,0,
                   ((invertOrder)?+1.:-1) * 
                   layerThickness/2
                   + ((invertOrder)?-1.:+1) * 
                    gapThickness/2
                 ),
                 gapLV,             // its logical volume
                 name+"_Gap",       // its name
                 layerLV,           // its mother  volume
                 false,             // no boolean operation
                 0,                 // copy number
                 m_checkOverlaps);  // checking overlaps
  }// Loop over calorimeter layers

}


// --- Aditivo: construcao da camara de muons ---
//
// Estrutura (parede) + gap de gas. Recebe o Volume com isMuonChamber=true
// e cria:
//   <name>            : G4Tubs envelope (defaultMaterial)
//   <name>_Structure  : G4Tubs de "absorberMaterial" (parede)
//   <name>_Gas        : G4Tubs de "gasMixture" (gap sensivel)
// A G4Region <name> e registrada como root do volume-mae.
//
void DetectorConstruction_v1::ConstructMuonChamber( G4LogicalVolume *worldLV,
                                                    const Volume &volume,
                                                    G4Material *defaultMaterial,
                                                    G4Region *region )
{
  G4Material* structureMat = G4Material::GetMaterial(volume.absorberMaterial);
  G4Material* gasMat       = G4Material::GetMaterial(volume.gasMixture);

  if ( !structureMat || !gasMat ) {
    G4ExceptionDescription msg;
    msg << "Cannot retrieve muon chamber materials ("
        << volume.absorberMaterial << ", " << volume.gasMixture << ").";
    G4Exception("DetectorConstruction_v1::ConstructMuonChamber()",
                "MuonChamber0001", FatalException, msg);
  }

  const double structureT = (volume.absoThickness > 0) ? volume.absoThickness : 0.5*cm;
  const double gapT       = (volume.gapThickness  > 0) ? volume.gapThickness  : 1.0*cm;

  const double rMin  = volume.rMin;
  const double rMax  = volume.rMax;
  const double zSize = volume.zSize;

  // Envelope (Vacuum)
  G4VSolid* chamberS = new G4Tubs( volume.name,
                                    rMin, rMax,
                                    zSize/2,
                                    0*deg,
                                    (!m_cutOnPhi)?(360*deg):(235*deg) );

  G4LogicalVolume* chamberLV = new G4LogicalVolume( chamberS,
                                                     defaultMaterial,
                                                     volume.name );

  new G4PVPlacement( 0,
                     G4ThreeVector(volume.x, volume.y, volume.z),
                     chamberLV,
                     volume.name,
                     worldLV,
                     false,
                     0,
                     m_checkOverlaps );

  region->AddRootLogicalVolume(chamberLV);

  // Estrutura (parede)
  G4VSolid* structureS = new G4Tubs( volume.name + "_Structure",
                                      rMin, rMin + structureT,
                                      zSize/2,
                                      0*deg,
                                      (!m_cutOnPhi)?(360*deg):(235*deg) );

  G4LogicalVolume* structureLV = new G4LogicalVolume( structureS,
                                                       structureMat,
                                                       volume.name + "_Structure" );

  new G4PVPlacement( 0,
                     G4ThreeVector(0,0,0),
                     structureLV,
                     volume.name + "_Structure",
                     chamberLV,
                     false,
                     0,
                     m_checkOverlaps );

  // Gap de gas
  G4VSolid* gasS = new G4Tubs( volume.name + "_Gas",
                                rMin + structureT, rMin + structureT + gapT,
                                zSize/2,
                                0*deg,
                                (!m_cutOnPhi)?(360*deg):(235*deg) );

  G4LogicalVolume* gasLV = new G4LogicalVolume( gasS,
                                                 gasMat,
                                                 volume.name + "_Gas" );

  new G4PVPlacement( 0,
                     G4ThreeVector(0,0,0),
                     gasLV,
                     volume.name + "_Gas",
                     chamberLV,
                     false,
                     0,
                     m_checkOverlaps );

  // Vis attributes (opcional, por tipo de camara)
  G4VisAttributes* vis = nullptr;
  switch (volume.chamberType) {
    case 0: vis = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0)); break; // MDT
    case 1: vis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0)); break; // RPC
    case 2: vis = new G4VisAttributes(G4Colour(1.0, 0.0, 1.0)); break; // TGC
    default: vis = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5));
  }
  vis->SetForceSolid(true);
  chamberLV->SetVisAttributes(vis);
}


void DetectorConstruction_v1::ConstructSDandField(){

  if (m_useMagneticField){
    MSG_INFO("Set magnetic field")
    // Create global magnetic field messenger.
    // Uniform magnetic field is then created automatically if
    // the field value is not zero.
    G4ThreeVector fieldValue(0.0 , 0.0, 2*tesla);
    m_magFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
    m_magFieldMessenger->SetVerboseLevel(1);

    // Register the field messenger for deleting
    G4AutoDelete::Register(m_magFieldMessenger); 
  }

  // --- Aditivo: camara de muons ---
  // Stub: registra a intencao. A SD C++ (MuonSensitiveDetector) ainda nao
  // esta integrada neste repositorio; ver relatorio.
  SetupMuonSensitiveDetectors();
}



void DetectorConstruction_v1::SetupMuonSensitiveDetectors() {
  int n = 0;
  for (const auto &v : m_volumes) if (v.isMuonChamber) ++n;
  if (n > 0) {
    MSG_INFO("Muon chambers in geometry: " << n
             << " (SetupMuonSensitiveDetectors: SD C++ ainda nao integrada)");
  }
}



G4Region* DetectorConstruction_v1::GetRegion( std::string name ){
  auto reg = G4RegionStore::GetInstance()->GetRegion(name);
  return reg ? reg : new G4Region(name);
}
