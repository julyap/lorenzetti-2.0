// ============================================================================
// Arquivo: MuonShapes.cxx
// Autora: Julya Pacheco Ferreira
// Descrição: Implementação do MuonShapes (AlgTool). Recebe um MuonCluster
//            e calcula:
//              - variáveis de tracking (charge, pT, p, d0, z0, chi2 de
//                track) via performTracking, com estimativas auxiliares
//                (estimatePt, estimateCharge, estimateD0, estimateZ0) e
//                um fit de hélice (fitHelix);
//              - variáveis de qualidade (fracMax, chi2, q/p significance,
//                rho') e cortes de qualidade (Loose, Medium, Tight).
//            Os resultados são armazenados no próprio cluster.
// ============================================================================

#include "MuonShapes.h"
#include "GaugiKernel/MsgStream.h"
#include "GaugiKernel/StatusCode.h"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include <cmath>
#include <algorithm>
#include <limits>

using namespace SG;
using namespace Gaugi;


MuonShapes::MuonShapes( std::string name ) :
  AlgTool(),
  m_doForwardMoments(true),
  m_magneticField(2.0),        // 2 Tesla (campo magnético do ATLAS)
  m_minPointsForFit(3),
  m_maxD0(100.0 * mm),         // 100 mm máximo para d0
  m_maxZ0(300.0 * mm)          // 300 mm máximo para z0
{
  // --- Declaração de propriedades configuráveis -------------------
  declareProperty("MagneticField", m_magneticField);
  declareProperty("MinPointsForFit", m_minPointsForFit);
  declareProperty("MaxD0", m_maxD0);
  declareProperty("MaxZ0", m_maxZ0);
}

//!=====================================================================

StatusCode MuonShapes::initialize()
{
  return StatusCode::SUCCESS;
}

//!=====================================================================

StatusCode MuonShapes::finalize()
{
  return StatusCode::SUCCESS;
}

// ============================================================
// TRACKING METHODS - NOVOS
// ============================================================

void MuonShapes::performTracking(xAOD::MuonCluster* clus) const
{
  const auto& cells = clus->cells();

  // Se não tiver células suficientes, usa valores padrão
  if (cells.size() < m_minPointsForFit) {
    clus->setCharge(0);
    clus->setPt(clus->totalEnergy());
    clus->setD0(0.0);
    clus->setZ0(0.0);
    clus->setTrackChi2(999.0);
    return;
  }

  // Coleta pontos das células
  std::vector<TVector3> points;
  for (const auto cell : cells) {
    TVector3 point(cell->x(), cell->y(), cell->z());
    points.push_back(point);
  }

  // Estima parâmetros da track
  float pt = estimatePt(clus);
  float d0 = estimateD0(clus);
  float z0 = estimateZ0(clus);
  float trackChi2 = 0.0;

  // Tenta fazer o fit da hélice (melhor estimativa)
  if (cells.size() >= 3) {
    fitHelix(points, pt, d0, z0, trackChi2);
  }

  // Aplica cortes de qualidade
  if (trackChi2 > 100.0 || std::abs(d0) > m_maxD0 || std::abs(z0) > m_maxZ0) {
    trackChi2 = 999.0;
  }

  // Estima carga baseado na curvatura
  int charge = estimateCharge(clus, pt);

  // Calcula eta e phi
  float theta = 2.0 * std::atan(std::exp(-clus->eta()));
  float p = pt / std::sin(theta);

  // Armazena os resultados no cluster
  clus->setCharge(charge);
  clus->setPt(pt);
  clus->setP(p);
  clus->setD0(d0);
  clus->setZ0(z0);
  clus->setTrackChi2(trackChi2);

  MSG_DEBUG("Tracking: pt=" << pt/GeV << " GeV, charge=" << charge
            << ", d0=" << d0/mm << " mm, z0=" << z0/mm << " mm");
}

//!=====================================================================

float MuonShapes::estimatePt(const xAOD::MuonCluster* clus) const
{
  // Estimativa simples: pT a partir da energia total e ângulo
  float theta = 2.0 * std::atan(std::exp(-clus->eta()));
  float sinTheta = std::sin(theta);
  float pt = clus->totalEnergy() * sinTheta;

  // Se tiver pelo menos 2 células, usa curvatura para estimar melhor
  const auto& cells = clus->cells();
  if (cells.size() >= 2) {
    // Calcula raio médio da trajetória
    float R_sum = 0.0;
    for (const auto cell : cells) {
      R_sum += std::sqrt(cell->x()*cell->x() + cell->y()*cell->y());
    }
    float R = R_sum / cells.size();

    // pT [GeV] = 0.3 * B [T] * R [m]
    float pt_from_curvature = 0.3 * m_magneticField * (R / 1000.0); // R em metros

    // Combina as duas estimativas
    if (pt_from_curvature > 0) {
      pt = (pt + pt_from_curvature) * 0.5;
    }
  }

  // CORREÇÃO AQUI: garantir que todos os argumentos são float
  float min_pt = 0.1f * GeV;      // explícitamente float
  float max_pt = 1000.0f * GeV;   // explícitamente float

  pt = std::max(min_pt, std::min(max_pt, pt));

  return pt;
}

//!=====================================================================

int MuonShapes::estimateCharge(const xAOD::MuonCluster* clus, float pt) const
{
  const auto& cells = clus->cells();

  // Precisa de pelo menos 3 pontos para determinar curvatura
  if (cells.size() < 3) return 1;  // default positivo

  // Calcula o produto vetorial para determinar direção da curvatura
  TVector3 p1(cells[0]->x(), cells[0]->y(), cells[0]->z());
  TVector3 p2(cells[1]->x(), cells[1]->y(), cells[1]->z());
  TVector3 p3(cells[2]->x(), cells[2]->y(), cells[2]->z());

  TVector3 v12 = p2 - p1;
  TVector3 v23 = p3 - p2;
  TVector3 cross = v12.Cross(v23);

  // O sinal da componente z indica a carga
  // Partículas positivas curvam em uma direção, negativas na oposta
  int charge = (cross.z() > 0) ? 1 : -1;

  return charge;
}

//!=====================================================================

float MuonShapes::estimateD0(const xAOD::MuonCluster* clus) const
{
  // d0 = parâmetro de impacto transversal (distância de aproximação mais próxima ao eixo z)
  const auto* seed = clus->seed();
  if (!seed) return 0.0;

  // Posição do seed no plano transverse
  float x = seed->x();
  float y = seed->y();
  float d0 = std::sqrt(x*x + y*y);

  // Adiciona sinal baseado na direção relativa ao momento
  float phi_cell = std::atan2(y, x);
  float phi_track = clus->phi();
  float delta_phi = std::abs(phi_cell - phi_track);
  if (delta_phi > M_PI) delta_phi = 2*M_PI - delta_phi;

  // Se a célula está atrás da direção do momento, d0 é negativo
  if (delta_phi > M_PI/2) d0 = -d0;

  return d0;
}

//!=====================================================================

float MuonShapes::estimateZ0(const xAOD::MuonCluster* clus) const
{
  // z0 = parâmetro de impacto longitudinal (distância em z ao ponto de maior aproximação)
  const auto* seed = clus->seed();
  if (!seed) return 0.0;

  // Estimativa simples: z0 é a posição z do seed
  // Em uma implementação mais completa, seria o z no ponto de maior aproximação
  return seed->z();
}

//!=====================================================================

bool MuonShapes::fitHelix(const std::vector<TVector3>& points,
                          float& pt, float& d0, float& z0, float& chi2) const
{
  if (points.size() < 3) return false;

  // Fit do círculo no plano transverse (x,y)
  // Usando método de mínimos quadrados: (x - xc)^2 + (y - yc)^2 = R^2
  // Expandindo: -2xc*x - 2yc*y + (xc^2+yc^2-R^2) = -(x^2+y^2)

  double sum_x = 0, sum_y = 0, sum_xy = 0;
  double sum_x2 = 0, sum_y2 = 0, sum_x3 = 0, sum_y3 = 0;
  double sum_x2y = 0, sum_xy2 = 0;
  int n = points.size();

  for (const auto& p : points) {
    double x = p.x();
    double y = p.y();
    sum_x += x;
    sum_y += y;
    sum_xy += x*y;
    sum_x2 += x*x;
    sum_y2 += y*y;
    sum_x3 += x*x*x;
    sum_y3 += y*y*y;
    sum_x2y += x*x*y;
    sum_xy2 += x*y*y;
  }

  // Resolve o sistema para encontrar (xc, yc, R)
  double A = n*sum_x2 - sum_x*sum_x;
  double B = n*sum_xy - sum_x*sum_y;
  double C = n*sum_y2 - sum_y*sum_y;
  double D = 0.5*(n*sum_x3 + n*sum_xy2 - sum_x*(sum_x2+sum_y2));
  double E = 0.5*(n*sum_y3 + n*sum_x2y - sum_y*(sum_x2+sum_y2));

  double det = A*C - B*B;
  if (std::abs(det) < 1e-6) return false;

  double xc = (D*C - B*E) / det;
  double yc = (A*E - B*D) / det;
  double R = std::sqrt(xc*xc + yc*yc + (sum_x2+sum_y2 - 2*xc*sum_x - 2*yc*sum_y)/n);

  // Calcula chi2 do fit
  chi2 = 0;
  for (const auto& p : points) {
    double dx = p.x() - xc;
    double dy = p.y() - yc;
    double residual = std::sqrt(dx*dx + dy*dy) - R;
    chi2 += residual*residual;
  }
  chi2 /= (n - 3);  // NDOF aproximado

  // Atualiza pT a partir da curvatura
  pt = 0.3 * m_magneticField * R;  // pT em GeV, R em metros
  pt = std::max(0.1f, std::min(1000.0f, pt));

  // Calcula d0 (distância de maior aproximação à origem)
  d0 = std::sqrt(xc*xc + yc*yc) - R;

  // Fit da linha reta para z vs s (comprimento do arco)
  if (n >= 2) {
    double sum_s = 0, sum_z = 0, sum_sz = 0, sum_s2 = 0;
    double s = 0;
    TVector3 prev = points[0];

    for (size_t i = 0; i < n; ++i) {
      if (i > 0) {
        s += (points[i] - prev).Mag();
        prev = points[i];
      }

      sum_s += s;
      sum_z += points[i].z();
      sum_sz += s * points[i].z();
      sum_s2 += s * s;
    }

    double slope = (n*sum_sz - sum_s*sum_z) / (n*sum_s2 - sum_s*sum_s);
    z0 = (sum_z - slope*sum_s) / n;
  } else {
    z0 = points[0].z();
  }

  return true;
}

// ============================================================
// MÉTODOS DE QUALIDADE ATUALIZADOS (depende do tracking)
// ============================================================

bool MuonShapes::evaluateLoose(float fracMax, float chi2, float qOverPSig,
                                float d0, float z0) const
{
  // Loose: qualidade básica
  if (fracMax < 0.1) return false;
  if (chi2 > 15.0) return false;
  if (std::abs(qOverPSig) > 10.0) return false;

  // Cortes adicionais de tracking
  if (std::abs(d0) > 50.0 * mm) return false;    // d0 < 50 mm
  if (std::abs(z0) > 200.0 * mm) return false;   // z0 < 200 mm

  return true;
}

//!=====================================================================

bool MuonShapes::evaluateMedium(float fracMax, float chi2, float qOverPSig,
                                 int nCells, float d0, float z0) const
{
  // Medium: seleção padrão ATLAS
  if (fracMax < 0.2) return false;
  if (chi2 > 10.0) return false;
  if (std::abs(qOverPSig) >= 7.0) return false;
  if (nCells < 1) return false;

  // Cortes de tracking mais restritivos
  if (std::abs(d0) > 20.0 * mm) return false;    // d0 < 20 mm
  if (std::abs(z0) > 100.0 * mm) return false;   // z0 < 100 mm

  return true;
}

//!=====================================================================

bool MuonShapes::evaluateTight(float fracMax, float chi2, float qOverPSig,
                                float rhoPrime, float d0, float z0) const
{
  // Tight: máxima pureza
  if (fracMax < 0.3) return false;
  if (chi2 >= 8.0) return false;

  float combinedMetric = std::abs(rhoPrime) + 0.1 * std::abs(qOverPSig);
  if (combinedMetric > 1.5) return false;

  if (std::abs(rhoPrime) > 0.3) return false;
  if (std::abs(qOverPSig) > 5.0) return false;

  // Cortes de tracking muito restritivos (muons prompt)
  if (std::abs(d0) > 5.0 * mm) return false;     // d0 < 5 mm
  if (std::abs(z0) > 20.0 * mm) return false;    // z0 < 20 mm

  return true;
}

// ============================================================
// MÉTODOS EXISTENTES ATUALIZADOS
// ============================================================

StatusCode MuonShapes::execute( SG::EventContext &/*ctx*/, Gaugi::EDM *edm ) const
{
  MSG_DEBUG("Calculate muon quality variables with tracking");

  // Converte o EDM para o cluster
  auto *clus = static_cast<xAOD::MuonCluster*>(edm);
  if (!clus) {
    MSG_WARNING("Failed to cast to MuonCluster");
    return StatusCode::FAILURE;
  }

  const auto& cells = clus->cells();
  float totalEnergy = clus->totalEnergy();
  int nCells = cells.size();

  // ============================================================
  // TRACKING - NOVO: Calcula charge, pt, d0, z0
  // ============================================================
  performTracking(clus);

  // ============================================================
  // CÁLCULO DAS VARIÁVEIS DE QUALIDADE (ATLAS 2016)
  // ============================================================

  // 1. fracMax
  float maxCellEnergy = 0.0;
  for (const auto cell : cells) {
    if (cell->m_energy > maxCellEnergy) maxCellEnergy = cell->m_energy;
  }
  float fracMax = (totalEnergy > 0) ? maxCellEnergy / totalEnergy : 0.0;

  // 2. χ² do cluster
  float chi2 = calculateChi2(clus);

  // 3. q/p significance
  float qOverPSignificance = calculateQOverPSignificance(clus);

  // 4. ρ' (rho prime)
  float rhoPrime = calculateRhoPrime(clus);

  // Pega os valores do tracking
  float d0 = clus->d0();
  float z0 = clus->z0();

  // ============================================================
  // APLICAÇÃO DOS CORTES COM TRACKING
  // ============================================================

  bool isVeryLoose = (totalEnergy > 0.01 * GeV);  // > 10 MeV

  bool isLoose = evaluateLoose(fracMax, chi2, qOverPSignificance, d0, z0);
  bool isMedium = evaluateMedium(fracMax, chi2, qOverPSignificance, nCells, d0, z0);
  bool isTight = evaluateTight(fracMax, chi2, qOverPSignificance, rhoPrime, d0, z0);

  // ============================================================
  // ARMAZENA OS RESULTADOS NO CLUSTER
  // ============================================================

  clus->setFracMax(fracMax);
  clus->setChi2(chi2);
  clus->setQOverPSignificance(qOverPSignificance);
  clus->setRhoPrime(rhoPrime);

  clus->setIsVeryLoose(isVeryLoose);
  clus->setIsLoose(isLoose);
  clus->setIsMedium(isMedium);
  clus->setIsTight(isTight);

  int quality = -1;
  if (isTight) quality = 3;
  else if (isMedium) quality = 2;
  else if (isLoose) quality = 1;
  else if (isVeryLoose) quality = 0;
  clus->setQuality(quality);

  MSG_INFO("Muon cluster: eta=" << clus->eta() << ", phi=" << clus->phi()
           << " | energy=" << totalEnergy/GeV << " GeV"
           << " | fracMax=" << fracMax
           << " | charge=" << clus->charge()
           << " | pt=" << clus->pt()/GeV << " GeV"
           << " | d0=" << d0/mm << " mm"
           << " | z0=" << z0/mm << " mm"
           << " | quality=" << quality);

  return StatusCode::SUCCESS;
}

// ============================================================
// MÉTODOS EXISTENTES (mantidos iguais)
// ============================================================

float MuonShapes::calculateChi2(const xAOD::MuonCluster* clus) const
{
  const auto& cells = clus->cells();
  if (cells.size() < 2) return 0.0;

  float totalEnergy = clus->totalEnergy();
  if (totalEnergy <= 0) return 999.0;

  float meanEnergy = totalEnergy / cells.size();
  float chi2 = 0.0;

  for (const auto cell : cells) {
    float diff = cell->m_energy - meanEnergy;
    chi2 += (diff * diff) / meanEnergy;
  }

  chi2 = chi2 / cells.size();
  return std::min(chi2, 100.0f);
}

//!=====================================================================

float MuonShapes::calculateQOverPSignificance(const xAOD::MuonCluster* clus) const
{
  const auto& cells = clus->cells();
  if (cells.size() < 2) return 0.0;

  float totalEnergy = clus->totalEnergy();
  float maxEnergy = 0.0;
  for (const auto cell : cells) {
    if (cell->m_energy > maxEnergy) maxEnergy = cell->m_energy;
  }

  float fracMax = (totalEnergy > 0) ? maxEnergy / totalEnergy : 0.0;
  float significance = 10.0 * (1.0 - fracMax);

  return significance;
}

//!=====================================================================

float MuonShapes::calculateRhoPrime(const xAOD::MuonCluster* clus) const
{
  const auto& cells = clus->cells();
  if (cells.size() < 2) return 0.0;

  float totalEnergy = clus->totalEnergy();
  if (totalEnergy <= 0) return 1.0;

  float meanX = 0.0, meanY = 0.0, meanZ = 0.0;
  for (const auto cell : cells) {
    float w = cell->m_energy / totalEnergy;
    meanX += w * cell->x();
    meanY += w * cell->y();
    meanZ += w * cell->z();
  }

  float variance = 0.0;
  for (const auto cell : cells) {
    float w = cell->m_energy / totalEnergy;
    float dx = cell->x() - meanX;
    float dy = cell->y() - meanY;
    float dz = cell->z() - meanZ;
    variance += w * (dx*dx + dy*dy + dz*dz);
  }

  return std::min(1.0f, variance / 10000.0f);
}
