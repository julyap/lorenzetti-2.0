#ifndef CaloClusterMaker_h
#define CaloClusterMaker_h

#include "CaloCell/enumeration.h"
#include "GaugiKernel/StatusCode.h"
#include "GaugiKernel/DataHandle.h"
#include "GaugiKernel/Algorithm.h"
#include "CaloCell/CaloCellContainer.h"
#include "CaloCluster/CaloClusterContainer.h"
#include "EventInfo/SeedContainer.h"
#include "ShowerShapes.h"
#include "Rtypes.h"  // <-- necessário para ClassDef

class CaloClusterMaker : public Gaugi::Algorithm
{

  public:
    /** Constructor **/
    CaloClusterMaker( std::string );
    virtual ~CaloClusterMaker();

    virtual StatusCode initialize() override;
    virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode execute( SG::EventContext &ctx , const G4Step *step) const override;
    virtual StatusCode execute( SG::EventContext &ctx , int /*evt*/ ) const override;
    virtual StatusCode post_execute( SG::EventContext &ctx ) const override;
    virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;
    virtual StatusCode finalize() override;

  private:

    void fillCluster( SG::EventContext &ctx,  xAOD::CaloCluster *clus, std::string key ) const;
    float dR( float eta1, float phi1, float eta2, float phi2 ) const;
    float calculateClusterDeltaEta(const std::vector<const xAOD::CaloCell*>& cells) const;
    float calculateClusterDeltaPhi(const std::vector<const xAOD::CaloCell*>& cells) const;

    // input keys
    std::string m_cellsKey; 
    std::string m_seedKey;
    // output keys
    std::string m_clusterKey;

    float m_etaWindow;
    float m_phiWindow;
    bool m_doForwardMoments;
    std::string m_histPath;

    ShowerShapes *m_showerShapes;
    float m_minCenterEnergy;

    ClassDef(CaloClusterMaker, 1)  // <- macro do ROOT
};

#endif

