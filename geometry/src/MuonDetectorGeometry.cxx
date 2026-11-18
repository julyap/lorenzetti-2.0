#include "MuonDetectorGeometry.h"

// Definição das constantes estáticas
const float MuonDetectorGeometry::m_phi_min = -M_PI;
const float MuonDetectorGeometry::m_twopi = 2 * M_PI;
const float MuonDetectorGeometry::m_phi_max = M_PI;

float MuonDetectorGeometry::fix(float phi)
{
    while (phi < m_phi_min) {
        phi += m_twopi;
    }
    while (phi >= m_phi_max) {
        phi -= m_twopi;
    }
    return phi;
}

float MuonDetectorGeometry::diff(float phi1, float phi2)
{
    float difference = phi1 - phi2;
    return fix(difference);
}
