#ifndef RootStreamAODReader_h
#define RootStreamAODReader_h

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/MsgStream.h"
#include "CaloCell/enumeration.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/Seed.h"
#include "TruthParticle/TruthParticle.h"

class RootStreamAODReader : public Gaugi::Algorithm
{

  public:
    /** Constructor **/
    RootStreamAODReader( std::string );

    virtual ~RootStreamAODReader();

    virtual StatusCode initialize() override;

    virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;

    virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;

    virtual StatusCode execute( SG::EventContext &ctx , const G4Step *step) const override;

    virtual StatusCode execute( SG::EventContext &ctx , int evt ) const override;

    virtual StatusCode post_execute( SG::EventContext &ctx ) const override;

    virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;

    virtual StatusCode finalize() override;

  private:

    StatusCode deserialize( int evt, SG::EventContext &ctx ) const;

    template <class T> void InitBranch(TTree* fChain, std::string branch_name, T* param) const;

    // --- Inputs (OFICIAL, PRESERVADOS) ---
    std::string m_eventKey;
    std::string m_truthKey;
    std::string m_seedsKey;
    std::string m_ringerKey;
    std::string m_ringerL0Key;
    std::string m_clusterKey;
    std::string m_electronKey;
    std::string m_truthCellsKey;
    std::string m_truthClusterKey;
    std::string m_truthRingerKey;
    std::string m_truthElectronKey;

    // --- Aditivo: muon ---
    std::string m_muonKey;
    std::string m_muonClusterKey;
    // ===== CORRIGIDO: chaves para MuonChamberCellContainer e descriptor =====
    std::string m_muonCellsKey;
    std::string m_muonDescriptorKey;

    // --- Output ---
    std::string m_ntupleName;
    std::string m_inputFile;

    // --- Vetores de ringer (OFICIAL, PRESERVADOS) ---
    std::vector<std::string> m_ringerKeys;
    std::vector<std::string> m_truthRingerKeys;

    // --- Flags ---
    bool m_enableCaloOutput;   // aditivo (default true)
    bool m_enableMuonOutput;   // aditivo (default true)
    // ===== CORRIGIDO: controlar se as MuonChamberCells sao deserializadas =====
    bool m_enableMuonCellsOutput;  // aditivo (default true; use false se nao houver branch)

    int m_outputLevel;
};

#endif
