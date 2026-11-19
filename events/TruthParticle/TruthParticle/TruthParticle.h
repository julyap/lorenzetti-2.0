#ifndef TruthParticle_h
#define TruthParticle_h

#include "GaugiKernel/EDM.h"
#include "GaugiKernel/macros.h"

namespace xAOD{
  
  class TruthParticle: public Gaugi::EDM
  {  
    public:

      /*! Constructor */
      TruthParticle();
      /*! Constructor */
      TruthParticle( int pdgid, int seedid, 
                     float e, float et, float eta, float phi, 
                     float px, float py, float pz, 
                     float vx, float vy, float vz); // production vertex position
                      
      /*! Destructor */
      ~TruthParticle()=default;
      
      /*! The eta center of the cluster given by the simulation (RoI) */ 
      PRIMITIVE_SETTER_AND_GETTER( float, m_eta, setEta, eta );
      /*! The phi center of the cluster given by the simulation (RoI) */ 
      PRIMITIVE_SETTER_AND_GETTER( float, m_phi, setPhi, phi );
      /*! set/get energy */
      PRIMITIVE_SETTER_AND_GETTER( float, m_e, setE, e );
      /*! set/get transverse energy */
      PRIMITIVE_SETTER_AND_GETTER( float, m_et, setEt, et );
      /*! set/get particle identification */
      PRIMITIVE_SETTER_AND_GETTER( int, m_pdgid, setPdgid, pdgid );
      
      PRIMITIVE_SETTER_AND_GETTER( int, m_seedid, setSeedid, seedid );

      PRIMITIVE_SETTER_AND_GETTER( float, m_px, setPx, px );
      
      PRIMITIVE_SETTER_AND_GETTER( float, m_py, setPy, py );

      PRIMITIVE_SETTER_AND_GETTER( float, m_pz, setPz, pz );

      /*! set/get vertex position */
      PRIMITIVE_SETTER_AND_GETTER( float, m_vtx_x, setVx, vx );

      PRIMITIVE_SETTER_AND_GETTER( float, m_vtx_y, setVy, vy );

      PRIMITIVE_SETTER_AND_GETTER( float, m_vtx_z, setVz, vz );

      // ============================================================
      // GETTERS PARA AS NOVAS VARIÁVEIS DO PAR
      // ============================================================
      int pair_pdg1() const { return m_pair_pdg1; }
      int pair_pdg2() const { return m_pair_pdg2; }
      float pair_pt1() const { return m_pair_pt1; }
      float pair_pt2() const { return m_pair_pt2; }
      float pair_eta1() const { return m_pair_eta1; }
      float pair_eta2() const { return m_pair_eta2; }
      float pair_phi1() const { return m_pair_phi1; }
      float pair_phi2() const { return m_pair_phi2; }
      float pair_delta_eta() const { return m_pair_delta_eta; }
      float pair_delta_phi() const { return m_pair_delta_phi; }
      float pair_delta_R() const { return m_pair_delta_R; }
      float pair_mass() const { return m_pair_mass; }
      float pair_pt() const { return m_pair_pt; }
      float pair_eta() const { return m_pair_eta; }
      float pair_phi() const { return m_pair_phi; }
      int pair_seedid() const { return m_pair_seedid; }

      // ============================================================
      // SETTERS PARA AS NOVAS VARIÁVEIS
      // ============================================================
      void set_pair_pdg1(int v) { m_pair_pdg1 = v; }
      void set_pair_pdg2(int v) { m_pair_pdg2 = v; }
      void set_pair_pt1(float v) { m_pair_pt1 = v; }
      void set_pair_pt2(float v) { m_pair_pt2 = v; }
      void set_pair_eta1(float v) { m_pair_eta1 = v; }
      void set_pair_eta2(float v) { m_pair_eta2 = v; }
      void set_pair_phi1(float v) { m_pair_phi1 = v; }
      void set_pair_phi2(float v) { m_pair_phi2 = v; }
      void set_pair_delta_eta(float v) { m_pair_delta_eta = v; }
      void set_pair_delta_phi(float v) { m_pair_delta_phi = v; }
      void set_pair_delta_R(float v) { m_pair_delta_R = v; }
      void set_pair_mass(float v) { m_pair_mass = v; }
      void set_pair_pt(float v) { m_pair_pt = v; }
      void set_pair_eta(float v) { m_pair_eta = v; }
      void set_pair_phi(float v) { m_pair_phi = v; }
      void set_pair_seedid(int v) { m_pair_seedid = v; }

    private:

      int m_pdgid;
      int m_seedid;
      float m_e;
      float m_et;
      float m_eta;
      float m_phi;
      float m_px;
      float m_py;
      float m_pz;
      // vertex position
      float m_vtx_x;      
      float m_vtx_y;
      float m_vtx_z;
      
      // ============================================================
      // NOVAS VARIÁVEIS PARA PARES DE PARTÍCULAS
      // ============================================================
      int m_pair_pdg1;
      int m_pair_pdg2;
      float m_pair_pt1;
      float m_pair_pt2;
      float m_pair_eta1;
      float m_pair_eta2;
      float m_pair_phi1;
      float m_pair_phi2;
      float m_pair_delta_eta;
      float m_pair_delta_phi;
      float m_pair_delta_R;
      float m_pair_mass;
      float m_pair_pt;
      float m_pair_eta;
      float m_pair_phi;
      int m_pair_seedid;

  };
}
#endif
