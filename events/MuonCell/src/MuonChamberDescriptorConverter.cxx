// ============================================================================
// Arquivo: src/MuonChamberDescriptorConverter.cxx
// Descrição: Implementação da classe xAOD::MuonChamberDescriptorConverter,
//            responsável por converter entre:
//              - MuonChamberDescriptor* → MuonChamberDescriptor_t
//                (serialização, incluindo acumulação de truthEnergy,
//                hitCount, clusterSize e driftTime médios a partir dos
//                BCIDs, e cópia do pulse);
//              - MuonChamberDescriptor_t → MuonChamberDescriptor*
//                (desserialização, criando um novo descriptor e
//                preenchendo os campos de operação, energia estimada,
//                energia de verdade e hits no BCID central).
// ============================================================================

#include "MuonCell/MuonChamberDescriptorConverter.h"

namespace xAOD {

//==============================================================================
// Serialização: MuonChamberDescriptor* → MuonChamberDescriptor_t
//   - Copia posição/dimensões, identificação (hash, chamberType, sampling,
//     layer, region) e configuração BCID (bcid_start, bcid_end,
//     bc_duration)
//   - Copia parâmetros de operação (efficiency, noiseLevel,
//     timeResolution, spatialResolution, driftVelocity, operatingVoltage,
//     gasMixture)
//   - Copia energia estimada (e)
//   - Acumula truthEnergy (soma de edep por BCID), hitCount, clusterSize
//     e driftTime médios
//   - Copia a forma de pulso (pulse)
//==============================================================================
bool MuonChamberDescriptorConverter::convert(const MuonChamberDescriptor* desc,
                                            MuonChamberDescriptor_t& desc_t)
{
    if (!desc) return false;

    // Copiar posição e dimensões
    desc_t.x = desc->x();
    desc_t.y = desc->y();
    desc_t.z = desc->z();
    desc_t.dx = desc->dx();
    desc_t.dy = desc->dy();
    desc_t.dz = desc->dz();

    // Copiar identificação
    desc_t.hash = desc->hash();
    desc_t.chamberType = desc->chamberType();
    desc_t.sampling = desc->sampling();
    desc_t.layer = desc->layer();
    desc_t.region = desc->region();

    // Copiar configuração BCID
    desc_t.bcid_start = desc->bcid_start();
    desc_t.bcid_end = desc->bcid_end();
    desc_t.bc_duration = desc->bc_duration();

    // Copiar parâmetros de operação
    desc_t.efficiency = desc->efficiency();
    desc_t.noiseLevel = desc->noiseLevel();
    desc_t.timeResolution = desc->timeResolution();
    desc_t.spatialResolution = desc->spatialResolution();
    desc_t.driftVelocity = desc->driftVelocity();
    desc_t.operatingVoltage = desc->operatingVoltage();
    desc_t.gasMixture = desc->gasMixture();

    // Copiar energia estimada (do filtro ótimo)
    desc_t.energy = desc->e();

    // Calcular energia verdadeira e hits
    desc_t.truthEnergy = 0.0f;
    desc_t.hitCount = 0;
    desc_t.clusterSize = 0;
    desc_t.driftTime = 0.0f;

    int start = desc->bcid_start();
    int end = desc->bcid_end();
    int validHits = 0;

    for (int bcid = start; bcid <= end; ++bcid) {
        // Energia verdadeira acumulada
        desc_t.truthEnergy += desc->edep(bcid);

        // Contagem de hits
        int hits = desc->hitCount(bcid);
        desc_t.hitCount += hits;

        // Tamanho do cluster (média) e drift time (média)
        if (hits > 0) {
            desc_t.clusterSize += desc->clusterSize(bcid);
            desc_t.driftTime += desc->driftTime(bcid);
            validHits++;
        }
    }

    // Média do cluster size e drift time
    if (validHits > 0) {
        desc_t.clusterSize /= validHits;
        desc_t.driftTime /= validHits;
    }

    // Copiar forma de pulso
    const auto& pulse = desc->pulse();
    desc_t.pulse.resize(pulse.size());
    std::copy(pulse.begin(), pulse.end(), desc_t.pulse.begin());

    return true;
}

//==============================================================================
// Desserialização: MuonChamberDescriptor_t → MuonChamberDescriptor*
//   - Cria um novo descriptor com posição/dimensões e identificação
//   - Configura os parâmetros de operação
//   - Configura energia estimada (e), energia de verdade (edep), hitCount,
//     clusterSize e driftTime no BCID central
//   - Copia a forma de pulso (pulse)
//==============================================================================
bool MuonChamberDescriptorConverter::convert(const MuonChamberDescriptor_t& desc_t,
                                            MuonChamberDescriptor*& desc)
{
    // Este método cria um novo MuonChamberDescriptor a partir da struct
    desc = new MuonChamberDescriptor(
        desc_t.x, desc_t.y, desc_t.z,
        desc_t.dx, desc_t.dy, desc_t.dz,
        desc_t.hash,
        desc_t.chamberType,
        desc_t.sampling,
        desc_t.layer,
        desc_t.region,
        desc_t.bc_duration,
        desc_t.bcid_start,
        desc_t.bcid_end
    );

    if (!desc) return false;

    // Copiar parâmetros de operação
    desc->setEfficiency(desc_t.efficiency);
    desc->setNoiseLevel(desc_t.noiseLevel);
    desc->setTimeResolution(desc_t.timeResolution);
    desc->setSpatialResolution(desc_t.spatialResolution);
    desc->setDriftVelocity(desc_t.driftVelocity);
    desc->setOperatingVoltage(desc_t.operatingVoltage);
    desc->setGasMixture(desc_t.gasMixture);

    // Configurar energia estimada
    desc->setE(desc_t.energy);

    // Configurar energia verdadeira e hits (no BCID central, por exemplo)
    int center_bcid = (desc_t.bcid_start + desc_t.bcid_end) / 2;
    desc->edep(center_bcid, desc_t.truthEnergy);
    desc->setHitCount(center_bcid, desc_t.hitCount);
    desc->setClusterSize(center_bcid, desc_t.clusterSize);
    desc->setDriftTime(center_bcid, desc_t.driftTime);

    // Copiar pulso
    desc->setPulse(desc_t.pulse);

    return true;
}

} // namespace xAOD
