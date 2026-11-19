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
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4RegionStore.hh"
#include "G4UserLimits.hh"          // ← NOVO: para G4UserLimits
#include <string>
#include <sstream>

namespace{
template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}
}

G4ThreadLocal G4GlobalMagFieldMessenger* DetectorConstruction_v1::m_magFieldMessenger = nullptr;


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
                                        double electronCut,
                                        double positronCut,
                                        double gammaCut,
                                        double photonCut,
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
  v.muonCut          = muonCut;
  v.isMuonChamber    = false;
  v.chamberType      = 0;
  v.gasMixture       = "";
  v.operatingVoltage = 0.0;
  v.gasPressure      = 1.0;

  m_volumes.push_back(v);
}



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
  v.plates           = 0;
  v.absorberMaterial = chamberStructure;
  v.gapMaterial      = gasMixture;
  v.nofLayers        = 1;
  v.rMin             = rMin;
  v.rMax             = rMax;
  v.zSize            = zSize;
  v.x                = 0;
  v.y                = 0;
  v.z                = zPos;

  // ============================================================
  // CUTS CORRIGIDOS: electronCut e o que controla a ionizacao
  // ============================================================
  v.electronCut      = 0.001 * mm;   // 1 um
  v.positronCut      = 0.001 * mm;   // 1 um
  v.gammaCut         = 0.01  * mm;   // 10 um
  v.photonCut        = 0.01  * mm;   // 10 um
  v.muonCut          = muonCut;      // nao critico para ionizacao

  v.isMuonChamber    = true;
  v.chamberType      = chamberType;
  v.gasMixture       = gasMixture;
  v.operatingVoltage = operatingVoltage;
  v.gasPressure      = gasPressure;

  // ============================================================
  // GEOMETRIA CORRIGIDA: gap e estrutura por tipo de camara
  // ============================================================
  switch (chamberType) {
    case 0:  // MDT: tubo de Ar/CO2, diametro 29.2 mm
      v.gapThickness  = 2 * 14.6 * mm;  // 29.2 mm
      v.absoThickness = 0.4 * mm;       // parede de Al
      break;
    case 1:  // RPC: gap de 2 mm
      v.gapThickness  = 2.0 * mm;
      v.absoThickness = 2.0 * mm;       // placa resistiva
      break;
    case 2:  // TGC: gap de 2.8 mm
      v.gapThickness  = 2.8 * mm;
      v.absoThickness = 1.4 * mm;       // catodo/anodo
      break;
    default:
      v.gapThickness  = 2.0 * mm;
      v.absoThickness = 1.0 * mm;
  }

  m_volumes.push_back(v);
}


G4VPhysicalVolume* DetectorConstruction_v1::Construct()
{
  DefineMaterials();

  G4Material* defaultMaterial  = G4Material::GetMaterial("Vacuum");

  G4VSolid* worldS = new G4Tubs( "World",
                                 0,
                                 11*m,
                                 15*m,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg) );

  G4LogicalVolume* worldLV = new G4LogicalVolume(
                                 worldS,
                                 defaultMaterial,
                                 "World");

  G4VPhysicalVolume* worldPV = new G4PVPlacement(
                 0,
                 G4ThreeVector(),
                 worldLV,
                 "World",
                 0,
                 false,
                 0,
                 m_checkOverlaps);

  for (auto &volume : m_volumes){

    MSG_INFO( "Creating Volume with name " << volume.name );

    if (volume.isMuonChamber) {
      auto region = GetRegion(volume.name);
      ConstructMuonChamber(worldLV, volume, defaultMaterial, region);


      G4ProductionCuts* cuts = new G4ProductionCuts();
      cuts->SetProductionCut(volume.gammaCut,    "gamma");
      cuts->SetProductionCut(volume.electronCut, "e-");
      cuts->SetProductionCut(volume.positronCut, "e+");
      cuts->SetProductionCut(volume.photonCut,   "proton");
      region->SetProductionCuts(cuts);
      continue;
    }

    if(volume.plates == 0){

      auto region = GetRegion(volume.name);

      CreateHorizontalPlates( worldLV,
                    volume.name,
                    defaultMaterial,
                    G4Material::GetMaterial(volume.absorberMaterial),
                    G4Material::GetMaterial(volume.gapMaterial),
                    volume.nofLayers,
                    volume.absoThickness,
                    volume.gapThickness,
                    volume.rMin,
                    volume.zSize,
                    G4ThreeVector(volume.x,volume.y,volume.z),
                    region );

      G4ProductionCuts* cuts=new G4ProductionCuts();
      cuts->SetProductionCut(volume.gammaCut,"gamma");
      cuts->SetProductionCut(volume.electronCut,"e-");
      cuts->SetProductionCut(volume.positronCut,"e+");
      cuts->SetProductionCut(volume.photonCut,"proton");
      region->SetProductionCuts(cuts);
      
    }else if (volume.plates == 1){

      auto region = GetRegion(volume.name);

      CreateVerticalPlates( worldLV,
                    volume.name,
                    defaultMaterial,
                    G4Material::GetMaterial(volume.absorberMaterial),
                    G4Material::GetMaterial(volume.gapMaterial),
                    volume.nofLayers,
                    volume.absoThickness,
                    volume.gapThickness,
                    volume.rMin,
                    volume.rMax,
                    volume.zSize,
                    G4ThreeVector(volume.x,volume.y,volume.z),
                    region );

      G4ProductionCuts* cuts=new G4ProductionCuts();
      cuts->SetProductionCut(volume.gammaCut,"gamma");
      cuts->SetProductionCut(volume.electronCut,"e-");
      cuts->SetProductionCut(volume.positronCut,"e+");
      cuts->SetProductionCut(volume.photonCut,"proton");
      region->SetProductionCuts(cuts);
    
    }else{
      MSG_FATAL("Volume type is not recognize. Abort!");
    }
  }

  return worldPV;
}


void DetectorConstruction_v1::DefineMaterials()
{
    G4NistManager* nistManager = G4NistManager::Instance();

    nistManager->FindOrBuildMaterial("G4_Cu");
    nistManager->FindOrBuildMaterial("G4_Pb");
    nistManager->FindOrBuildMaterial("G4_Fe");
    nistManager->FindOrBuildMaterial("G4_Al");
    nistManager->FindOrBuildMaterial("G4_Si");
    nistManager->FindOrBuildMaterial("G4_CESIUM_IODIDE");
    nistManager->FindOrBuildMaterial("G4_Ar");
    nistManager->FindOrBuildMaterial("G4_AIR");
    nistManager->FindOrBuildMaterial("G4_POLYSTYRENE");

    G4Element* elH = new G4Element("Hydrogen", "H", 1., 1.0079 * g/mole);
    G4Element* elC = new G4Element("Carbon",   "C", 6., 12.011 * g/mole);
    G4Element* elF = new G4Element("Fluorine", "F", 9., 18.998 * g/mole);
    G4Element* elO = new G4Element("Oxygen",   "O", 8., 15.999 * g/mole);

    if (!G4Material::GetMaterial("G4_CO2", false)) {
        G4Material* co2 = new G4Material(
            "G4_CO2", 1.842 * mg/cm3, 2,
            kStateGas, 293.15 * kelvin, 1.0 * atmosphere);
        co2->AddElement(elC, 1);
        co2->AddElement(elO, 2);
    }

    if (!G4Material::GetMaterial("G4_C2H2F4", false)) {
        G4Material* r134a = new G4Material(
            "G4_C2H2F4", 4.25 * mg/cm3, 3,
            kStateGas, 293.15 * kelvin, 1.0 * atmosphere);
        r134a->AddElement(elC, 2);
        r134a->AddElement(elH, 2);
        r134a->AddElement(elF, 4);
    }

    if (!G4Material::GetMaterial("PLASTIC SCINTILLATOR", false)) {
        G4Material* scinti = new G4Material(
            "PLASTIC SCINTILLATOR", 1.032 * g/cm3, 2);
        scinti->AddElement(elH, 8.5 * perCent);
        scinti->AddElement(elC, 91.5 * perCent);
    }

    G4double a, z, density;

    if (!G4Material::GetMaterial("liquidArgon", false)) {
        new G4Material("liquidArgon", z = 18., a = 39.95 * g/mole,
                       density = 1.390 * g/cm3);
    }

    if (!G4Material::GetMaterial("Vacuum", false)) {
        new G4Material("Vacuum", z = 1., a = 1.01 * g/mole,
                       density = universe_mean_density,
                       kStateGas, 2.73 * kelvin, 3.e-18 * pascal);
    }

    G4cout << "\n========== GEANT4 MATERIAL TABLE ==========\n"
           << *(G4Material::GetMaterialTable())
           << "\n===========================================\n"
           << G4endl;
}


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

  G4VSolid* calorimeterS = new G4Tubs( name,
                                 calorRmin,
                                 calorRmin+ nofLayers*layerThickness,
                                 calorZ/2,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg) );

  G4LogicalVolume* calorLV = new G4LogicalVolume( calorimeterS,
                                                  defaultMaterial,
                                                  name);
  new G4PVPlacement(
                 0,
                 center_pos,
                 calorLV,
                 name,
                 worldLV,
                 false,
                 0,
                 m_checkOverlaps);
  
  region->AddRootLogicalVolume(calorLV);

  for (G4int layer=0; layer < nofLayers; ++layer){

    G4VSolid* layerS = new G4Tubs(name+ "_Layer",
                                 calorRmin + layer*layerThickness,
                                 calorRmin + (layer+1)*layerThickness,
                                 calorZ/2,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg) );

    G4LogicalVolume* layerLV = new G4LogicalVolume(
                                                  layerS,
                                                  defaultMaterial,
                                                  name+"_Layer");

    new G4PVPlacement(
                 0,
                 G4ThreeVector(0,0,0),
                 layerLV,
                 name+"_Layer",
                 calorLV,
                 false,
                 0,
                 m_checkOverlaps);

    G4VSolid* absorverS = new G4Tubs( name+"_Abso",
                                 calorRmin + layer*(absoThickness + gapThickness) ,
                                 calorRmin + layer*(absoThickness + gapThickness) + absoThickness,
                                 calorZ/2,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg) );

    G4LogicalVolume* absorverLV = new G4LogicalVolume(
                                                  absorverS,
                                                  absorberMaterial,
                                                  name+"_Abso");

    new G4PVPlacement(
                 0,
                 G4ThreeVector(0,0,0),
                 absorverLV,
                 name+"_Abso",
                 layerLV,
                 false,
                 0,
                 m_checkOverlaps);

    G4VSolid* gapS = new G4Tubs( name+"_Gap",
                                 calorRmin + layer*(absoThickness + gapThickness) + absoThickness,
                                 calorRmin + (layer+1)*(absoThickness + gapThickness),
                                 calorZ/2,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg) ); 

    G4LogicalVolume* gapLV = new G4LogicalVolume(
                                                  gapS,
                                                  gapMaterial,
                                                  name+"_Gap");

    new G4PVPlacement(
                 0,
                 G4ThreeVector(0,0,0),
                 gapLV,
                 name+"_Gap",
                 layerLV,
                 false,
                 0,
                 m_checkOverlaps);
  }
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

  G4VSolid* calorimeterS = new G4Tubs( name,
                                 calorRmin,
                                 calorRmax,
                                 calorZ/2,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg) );

  G4LogicalVolume* calorLV = new G4LogicalVolume( calorimeterS,
                                                  defaultMaterial,
                                                  name);

  new G4PVPlacement(
                 0,
                 center_pos,
                 calorLV,
                 name,
                 worldLV,
                 false,
                 0,
                 m_checkOverlaps);
  
  region->AddRootLogicalVolume(calorLV);

  for (G4int layer=-nofLayers/2; layer <= nofLayers/2; ++layer){
    if ( layer == 0 && evenNofLayers){
      continue;
    }

    G4VSolid* layerS = new G4Tubs(name+ "_Layer",
                                 calorRmin,
                                 calorRmax,
                                 layerThickness/2,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg) );

    G4LogicalVolume* layerLV = new G4LogicalVolume(
                                                  layerS,
                                                  defaultMaterial,
                                                  name+"_Layer");

    const auto layerLVCenter = G4ThreeVector(0,0,
                   ((invertOrder)?-1.:+1.) * 
                   (sign(-layer)*((evenNofLayers)?0.5:0.) + (layer))
                   *layerThickness
                 );

    new G4PVPlacement(
                 0,
                 layerLVCenter,
                 layerLV,
                 name+"_Layer",
                 calorLV,
                 false,
                 0,
                 m_checkOverlaps);

    G4VSolid* absorverS = new G4Tubs( name+"_Abso",
                                 calorRmin,
                                 calorRmax,
                                 absoThickness/2,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg) );

    G4LogicalVolume* absorverLV = new G4LogicalVolume(
                                                  absorverS,
                                                  absorberMaterial,
                                                  name+"_Abso");

    new G4PVPlacement(
                 0,
                 G4ThreeVector(0,0,
                   ((invertOrder)?-1.:+1) * 
                   layerThickness/2
                   + ((invertOrder)?+1.:-1)/2 * 
                    absoThickness
                 ),
                 absorverLV,
                 name+"_Abso",
                 layerLV,
                 false,
                 0,
                 m_checkOverlaps);

    G4VSolid* gapS = new G4Tubs( name+"_Gap",
                                 calorRmin,
                                 calorRmax,
                                 gapThickness/2,
                                 0*deg,
                                 (!m_cutOnPhi)?(360*deg):(235*deg) ); 

    G4LogicalVolume* gapLV = new G4LogicalVolume(
                                                  gapS,
                                                  gapMaterial,
                                                  name+"_Gap");

    new G4PVPlacement(
                 0,
                 G4ThreeVector(0,0,
                   ((invertOrder)?+1.:-1) * 
                   layerThickness/2
                   + ((invertOrder)?-1.:+1) * 
                    gapThickness/2
                 ),
                 gapLV,
                 name+"_Gap",
                 layerLV,
                 false,
                 0,
                 m_checkOverlaps);
  }
}



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


  G4UserLimits* gasLimits = new G4UserLimits(0.05 * mm);
  gasLV->SetUserLimits(gasLimits);

  // Vis attributes
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
    G4ThreeVector fieldValue(0.0 , 0.0, 2*tesla);
    m_magFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
    m_magFieldMessenger->SetVerboseLevel(1);
    G4AutoDelete::Register(m_magFieldMessenger); 
  }

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
