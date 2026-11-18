// RootStreamESDMaker.h
#ifndef RootStreamESDMaker_h
#define RootStreamESDMaker_h

#include "CaloCell/enumeration.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/Seed.h"
#include "TruthParticle/TruthParticle.h"
#include "GaugiKernel/StatusCode.h"
#include "GaugiKernel/DataHandle.h"
#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/DataHandle.h"
#include <vector>
#include <set>

// Forward declarations
class TTree;

/**
 * @class RootStreamESDMaker
 * @brief Algorithm to stream ESD Data to a ROOT file.
 *
 * ESD contains detailed information including all Calorimeter Cells,
 * which makes it larger than AOD but necessary for full reconstruction study.
 * This algo serializes the Event Context into a TTree.
 *
 * --- Aditivo: muon ---
 * Also supports muon chamber cells. The muon cells are written to a separate
 * branch (MuonChamberCellContainer_<key>) when EnableMuonOutput is true.
 */
class RootStreamESDMaker : public Gaugi::Algorithm
{

  public:
    /** Constructor **/
    RootStreamESDMaker( std::string );

    virtual ~RootStreamESDMaker();

    virtual StatusCode initialize() override;

    virtual StatusCode bookHistograms( SG::EventContext &ctx ) const override;

    virtual StatusCode pre_execute( SG::EventContext &ctx ) const override;

    virtual StatusCode execute( SG::EventContext &ctx , const G4Step *step) const override;

    virtual StatusCode execute( SG::EventContext &ctx , int /*evt*/ ) const override;

    virtual StatusCode post_execute( SG::EventContext &ctx ) const override;

    virtual StatusCode fillHistograms( SG::EventContext &ctx ) const override;

    virtual StatusCode finalize() override;

  private:
    StatusCode serialize( SG::EventContext &ctx ) const;
    template <class T> void InitBranch(TTree* fChain, std::string branch_name, T* param) const;

    std::string m_ntupleName;

    // --- Oficiais: calorimetro ---
    std::string m_inputCellsKey;
    std::string m_inputCellsTruthKey;
    std::string m_outputCellsKey;
    std::string m_outputCellsTruthKey;

    // --- Aliases da sua versao (opcionais; se passados, tem precedencia) ---
    std::string m_inputCaloCellsKey;
    std::string m_inputTruthCaloCellsKey;
    std::string m_outputCaloCellsKey;
    // ===== CORRIGIDO: alias simetrico para output de truth =====
    std::string m_outputTruthCaloCellsKey;

    // --- Aditivo: muon ---
    std::string m_inputMuonCellsKey;
    std::string m_inputTruthMuonCellsKey;
    std::vector<std::string> m_inputMuonCellKeys;
    std::string m_outputMuonCellsKey;

    // --- Oficiais (comuns) ---
    std::string m_inputEventKey;
    std::string m_inputTruthKey;
    std::string m_inputSeedsKey;
    std::string m_outputEventKey;
    std::string m_outputTruthKey;
    std::string m_outputSeedsKey;

    // --- Oficiais: janelas de RoI ---
    float m_etaWindow;
    float m_phiWindow;
    // If true (default), only the cells inside the RoI around the truth particles are saved; if false, all cells
    bool m_onlyRoI;

    // --- Flags aditivas (default preservam o comportamento oficial) ---
    bool m_enableCaloOutput;
    bool m_enableMuonOutput;

    int m_outputLevel;
};

#endif
