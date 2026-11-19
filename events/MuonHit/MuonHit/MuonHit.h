// ============================================================================
// Arquivo: MuonHit/MuonHit/MuonHit.h
// Descrição: Declaração da classe xAOD::MuonHit, que representa um hit
//            individual de múon no EDM. Herda de Gaugi::EDM e armazena:
//              - Posição e dimensões: x, y, z, dx, dy;
//              - Identificação: hash, sampling, detector, region;
//              - Configuração BCID: bcid_start, bcid_end, bc_duration;
//              - Energia: energyDeposit (acumulador rápido) e mapas por BCID
//                (edep, tof, driftTime);
//              - Pulso (pulse) e tempo (time);
//              - Flag firstHit.
//            Fornece métodos fill() para preencher o hit a partir de um
//            G4Step, clear(), getters/setters por BCID e acesso aos mapas
//            para serialização.
// ============================================================================

#ifndef MuonHit_h
#define MuonHit_h

#include "../../MuonDefs/MuonDefs.h"
#include "GaugiKernel/EDM.h"
#include "GaugiKernel/macros.h"
#include "G4Step.hh"
#include "globals.hh"

#include <map>
#include <vector>

namespace xAOD {

  class MuonHit : public Gaugi::EDM
  {
    public:
      /** Construtor padrão **/
      MuonHit();

      /** Construtor principal **/
      MuonHit(
        float x, float y, float z,
        float dx, float dy,
        unsigned long int hash,
        MuonSampling sampling,
        MuonChamberType detector,
        MuonRegion region,
        float bc_duration,
        int bcid_start,
        int bcid_end,
        float energy
      );

      /** Destrutor **/
      ~MuonHit() = default;

      // --- Preenchimento a partir de um G4Step ------------------------
      void fill(const G4Step *);
      void fill(const G4Step *, float sampNoiseStd);
      void clear();

      // --- Macros para atributos (getters/setters automáticos) --------
      PRIMITIVE_SETTER_AND_GETTER(float, m_x, setX, x);
      PRIMITIVE_SETTER_AND_GETTER(float, m_y, setY, y);
      PRIMITIVE_SETTER_AND_GETTER(float, m_z, setZ, z);
      PRIMITIVE_SETTER_AND_GETTER(float, m_dx, setDeltaX, dx);
      PRIMITIVE_SETTER_AND_GETTER(float, m_dy, setDeltaY, dy);
      PRIMITIVE_SETTER_AND_GETTER(unsigned long int, m_hash, setHash, hash);
      PRIMITIVE_SETTER_AND_GETTER(MuonSampling, m_sampling, setSampling, sampling);
      PRIMITIVE_SETTER_AND_GETTER(MuonChamberType, m_detector, setDetector, detector);
      PRIMITIVE_SETTER_AND_GETTER(MuonRegion, m_region, setRegion, region);
      PRIMITIVE_SETTER_AND_GETTER(float, m_bc_duration, setBcDuration, bc_duration);
      PRIMITIVE_SETTER_AND_GETTER(int, m_bcid_start, setBcidStart, bcid_start);
      PRIMITIVE_SETTER_AND_GETTER(int, m_bcid_end, setBcidEnd, bcid_end);

      // --- Energia depositada (CORRIGIDO) ----------------------------
      // Retorna a energia acumulada (rápida) ou a soma do mapa (leitura)
      float energyDeposit() const;
      void setEnergyDeposit(float e) { m_energyDeposit = e; }

      // --- Tempo e pulso ---------------------------------------------
      float time() const { return m_timeValue; }
      void setTime(float t) { m_timeValue = t; }

      const std::vector<float>& pulse() const { return m_pulse; }
      void setPulse(const std::vector<float>& p) { m_pulse = p; }

      // --- Métodos extras (acesso por BCID) --------------------------
      float edep(int bcid) const;
      float tof(int bcid) const;
      float driftTime(int bcid) const;
      void set_edep(int bcid, float value);
      void set_tof(int bcid, float value);
      void set_driftTime(int bcid, float value);
      bool firstHit() const;

      // --- Métodos para serialização dos mapas ----------------------
      const std::map<int, float>& getEdepMap() const { return m_edep; }
      const std::map<int, float>& getTofMap() const { return m_tof; }
      const std::map<int, float>& getDriftTimeMap() const { return m_driftTime; }

      void addEdep(int bcid, float value) { m_edep[bcid] = value; }
      void addTof(int bcid, float value) { m_tof[bcid] = value; }
      void addDriftTime(int bcid, float value) { m_driftTime[bcid] = value; }

      // --- Compatibilidade antiga ------------------------------------
      unsigned long int id() const { return m_hash; }

    private:
      // Encontra o índice do BCID correspondente ao tempo fornecido
      int find(float value) const;

      // --- Vetores/mapas internos ------------------------------------
      std::vector<float> m_time;                  // tempos por BCID
      std::map<int, float> m_edep;                // energia por BCID
      std::map<int, float> m_tof;                 // time-of-flight por BCID
      std::map<int, float> m_driftTime;           // drift time por BCID

      bool m_firstHit;                            // flag de primeiro hit

      // --- Atributos geométricos e de identificação ------------------
      float m_x, m_y, m_z;
      float m_dx, m_dy;
      unsigned long int m_hash;
      MuonSampling m_sampling;
      MuonChamberType m_detector;
      MuonRegion m_region;
      int m_bcid_start;
      int m_bcid_end;
      float m_bc_duration;

      // --- Para compatibilidade com MuonPulseGenerator ---------------
      float m_energyDeposit;  // ← ACUMULADOR RÁPIDO
      float m_timeValue;
      std::vector<float> m_pulse;
  };

}

#endif
