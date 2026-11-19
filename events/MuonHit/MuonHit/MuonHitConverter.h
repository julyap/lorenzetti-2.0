// ============================================================================
// Arquivo: MuonHit/MuonHit/MuonHitConverter.h
// Descrição: Define a struct POD xAOD::MuonHit_t (usada para persistência
//            em ROOT) e a classe MuonHitConverter, responsável por converter
//            entre xAOD::MuonHit* e MuonHit_t.
//            A struct inclui:
//              - Campos básicos (sampling, detector, region, x, y, z, dx,
//                dy, bcid_start, bcid_end, bc_duration, energy, time, hash);
//              - Vetores para os mapas de BCID (bcid_values, edep_values,
//                tof_values, driftTime_values), permitindo persistir a
//                evolução temporal do sinal.
// ============================================================================

#ifndef MuonHitConverter_h
#define MuonHitConverter_h

#include "MuonHit/MuonHit.h"
#include <map>
#include <vector>

namespace xAOD{

    // Mapa de links para hits (usado para resolver índices)
    typedef std::map<const MuonHit *, int> muon_descriptor_links_t;

    // --- Struct POD serializável (usada para persistência em ROOT) ---
    struct MuonHit_t{
        int sampling;
        int detector;
        int region;
        float x;
        float y;
        float z;
        float dx;
        float dy;
        int bcid_start;
        int bcid_end;
        float bc_duration;
        float energy;
        float time;
        unsigned long int hash;

        // NOVOS CAMPOS PARA MAPAS DE BCID
        std::vector<int> bcid_values;
        std::vector<float> edep_values;
        std::vector<float> tof_values;
        std::vector<float> driftTime_values;
    };

    class MuonHitConverter{
        public:
            MuonHitConverter()=default;
            ~MuonHitConverter()=default;

            // Serialização: MuonHit* → MuonHit_t
            bool convert(const MuonHit *, MuonHit_t &);
            // Desserialização: MuonHit_t → MuonHit*
            bool convert( const MuonHit_t &, MuonHit *& );
    };
}

#endif
