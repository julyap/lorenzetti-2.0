
#ifndef RootStreamESDReader_h
#define RootStreamESDReader_h

#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/MsgStream.h"
#include "CaloCell/enumeration.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/Seed.h"
#include "TruthParticle/TruthParticle.h"

#include <string>
#include <vector>

/**
 * @class RootStreamESDReader
 * @brief Algorithm to read ESD (Event Summary Data) from a ROOT file.
 *
 * Reads detailed cell-level data and reconstructed objects from disk.
 *
 * Also supports reading muon chamber cells and descriptors.
 */
class RootStreamESDReader : public Gaugi::Algorithm
{

  public:

    RootStreamESDReader(std::string);

    virtual ~RootStreamESDReader();

    virtual StatusCode initialize() override;

    virtual StatusCode bookHistograms(SG::EventContext &ctx) const override;

    virtual StatusCode pre_execute(SG::EventContext &ctx) const override;

    virtual StatusCode execute(
      SG::EventContext &ctx,
      const G4Step *step
    ) const override;

    virtual StatusCode execute(
      SG::EventContext &ctx,
      int evt
    ) const override;

    virtual StatusCode post_execute(SG::EventContext &ctx) const override;

    virtual StatusCode fillHistograms(SG::EventContext &ctx) const override;

    virtual StatusCode finalize() override;


  private:

    StatusCode deserialize(int evt, SG::EventContext &ctx) const;

    template <class T>
    void InitBranch(
      TTree* fChain,
      std::string branch_name,
      T* param
    ) const;


    // ---------------------------------------------------------
    // Calorimeter
    // ---------------------------------------------------------

    std::string m_cellsKey;
    std::string m_cellsTruthKey;

    // Aliases opcionais
    std::string m_calocellsKey;
    std::string m_truthCaloCellsKey;


    // ---------------------------------------------------------
    // Muon
    // ---------------------------------------------------------

    std::string m_muonCellsKey;
    std::vector<std::string> m_muonCellKeys;


    // ---------------------------------------------------------
    // EventInfo e containers comuns
    // ---------------------------------------------------------

    // Chave de saída no StoreGate
    std::string m_eventKey;

    // Sufixo da branch de entrada no ROOT
    std::string m_inputEventBranchKey;

    std::string m_truthKey;
    std::string m_seedsKey;

    std::string m_inputFile;
    std::string m_ntupleName;


    // ---------------------------------------------------------
    // Flags
    // ---------------------------------------------------------

    bool m_enableCaloOutput;
    bool m_enableMuonOutput;

    int m_outputLevel;

};

#endif

