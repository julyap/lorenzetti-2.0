// TruthParticleConverter.h
#ifndef TruthParticleConverter_h
#define TruthParticleConverter_h

#include "TruthParticle/TruthParticle.h"

namespace xAOD{

    struct TruthParticle_t{
        int pdgid;
        int seedid;
        float e;
        float et;
        float eta;
        float phi;
        float px;
        float py;
        float pz;
        float vx; // vertex position x (prod_vx)
        float vy; // vertex position y
        float vz; // vertex position z
        
        // ============================================================
        // NOVAS VARIÁVEIS PARA PARES DE PARTÍCULAS
        // Serão preenchidas apenas para a primeira partícula do par
        // ============================================================
        int pair_pdg1;          // PDG da primeira partícula
        int pair_pdg2;          // PDG da segunda partícula
        float pair_pt1;         // pt da primeira partícula
        float pair_pt2;         // pt da segunda partícula
        float pair_eta1;        // eta da primeira partícula
        float pair_eta2;        // eta da segunda partícula
        float pair_phi1;        // phi da primeira partícula
        float pair_phi2;        // phi da segunda partícula
        float pair_delta_eta;   // Δη entre as partículas
        float pair_delta_phi;   // Δφ entre as partículas
        float pair_delta_R;     // ΔR entre as partículas
        float pair_mass;        // Massa invariante do par
        float pair_pt;          // pt total do par
        float pair_eta;         // eta do par
        float pair_phi;         // phi do par
        int pair_seedid;        // Seed ID do par
    };

    class TruthParticleConverter{
        public:
            TruthParticleConverter()=default;
            ~TruthParticleConverter()=default;

            bool convert(const TruthParticle *truth, TruthParticle_t &truth_t ) const;
            bool convert(const TruthParticle_t & , TruthParticle *&) const;
    };
}
#endif
