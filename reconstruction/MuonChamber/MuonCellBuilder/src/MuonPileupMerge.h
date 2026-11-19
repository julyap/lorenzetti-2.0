// ============================================================================
// Arquivo: MuonPileupMerge.h
// Descrição: Declaração da classe MuonPileupMerge (implementação em
//            MuonPileupMerge.cxx). Define a interface do algoritmo que
//            mescla hits e EventInfo de um arquivo ROOT de pileup com
//            os do evento principal.
// ============================================================================

#ifndef MUONPILEUPMERGE_H
#define MUONPILEUPMERGE_H

/** C++ includes **/
#include <string>
#include <map>
#include <vector>

/** ROOT includes **/
#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"

/** Framework includes **/
#include "GaugiKernel/Algorithm.h"
#include "GaugiKernel/Property.h"
#include "GaugiKernel/StatusCode.h"
#include "GaugiKernel/DataHandle.h"
#include "GaugiKernel/StoreGate.h"
#include "GaugiKernel/MsgStream.h"

/** xAOD includes **/
#include "MuonHit/MuonHit.h"
#include "MuonHit/MuonHitContainer.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventInfoContainer.h"


class MuonPileupMerge : public Gaugi::Algorithm
{
  public:
    MuonPileupMerge(std::string name);
    virtual ~MuonPileupMerge();

    // Ciclo de vida
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Execução (hooks opcionais)
    StatusCode pre_execute();
    StatusCode execute();
    StatusCode post_execute();

  private:

    // Método auxiliar
    template <class T>
    void InitBranch(TTree* fChain, std::string branch_name, T* param) const;

    // Propriedades
    std::string m_inputFile;
    std::string m_inputHitsKey;
    std::string m_outputHitsKey;
    std::string m_inputEventKey;
    std::string m_outputEventKey;
    int m_outputLevel;
    std::string m_ntupleName;
    int m_seed;

    mutable TRandom3 m_rng;
};

#endif
