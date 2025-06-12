#include "GenKernel/IAlgorithm.h"
#include <cmath>

using namespace Gaugi;
using namespace generator;
static const double c_light = 2.99792458e+8; // m/s

IAlgorithm::IAlgorithm(IGenerator* gen) :
    IMsgService(),
    PropertyService(),
    m_generator(gen)
{
    // Definindo as propriedades do algoritmo
    declareProperty("OutputLevel", m_outputLevel = 0);
    declareProperty("Sigma_t", m_sigma_t = 200 * 1e-12 /*pico segundos*/ * c_light /*m/s*/ * 1e+3 /*mm*/);
    declareProperty("Trunc_t", m_trunc_t = 10 * 1e-9 /*nano segundos*/ * c_light /*m/s*/ * 1e+3 /*mm*/);
    
    // Substituindo a flutuação no eixo Z pela flutuação associada ao J/psi
    declareProperty("Sigma_jpsi", m_sigma_jpsi = 0.3 /*GeV/c^2*/);  // Exemplo de valor para a flutuação do J/psi
    declareProperty("Trunc_jpsi", m_trunc_jpsi = 1.0 /*GeV/c^2*/);  // Limite para a flutuação do J/psi
    declareProperty( "Sigma_z"     , m_sigma_z = 56 /*milimiters*/                                           );
    declareProperty( "Trunc_z"     , m_trunc_z = 10 * 1e-9 /*nano seconds*/ * c_light /*m/s*/ * 1e+3 /*mm*/  );
    // Truncamento de valores muito altos
    declareProperty("Trunc_mu", m_trunc_mu = 300);
// Substituindo a flutuação do J/ψ pela do Upsilon
    declareProperty("Sigma_Upsilon", m_sigma_Upsilon = 0.3 /*GeV/c^2*/);  // Flutuação do Upsilon
    declareProperty("Trunc_Upsilon", m_trunc_Upsilon = 1.0 /*GeV/c^2*/);  // Limite para a flutuação do Upsilon
}



int IAlgorithm::poisson(double nAvg)
{
    // Número aleatório de Poisson.
    double rPoisson = m_generator->random_flat() * exp(nAvg);
    double rSum = 0.;
    double rTerm = 1.;

    // Calcula a soma dos termos e verifica se alcançou o valor do Poisson.
    for (int i = 0; i < m_trunc_mu;) {
        rSum += rTerm;
        if (rSum > rPoisson) return i;

        ++i;
        rTerm *= nAvg / i;
    }

    // Retorno de segurança.
    return m_trunc_mu;
}

float IAlgorithm::sample_z() const{
  return trunc_centered_gauss(m_sigma_z, m_trunc_z);
}


float IAlgorithm::sample_jpsi_decay() const
{
    // Gerando flutuações associadas ao decaimento do J/psi
    return trunc_centered_gauss(m_sigma_jpsi, m_trunc_jpsi);
}

float IAlgorithm::sample_t() const
{
    // Gerando flutuações associadas ao tempo
    return trunc_centered_gauss(m_sigma_t, m_trunc_t);
}

float IAlgorithm::sample_Upsilon_decay() const
{
    // Gerando flutuações associadas ao decaimento do Upsilon
    return trunc_centered_gauss(m_sigma_Upsilon, m_trunc_Upsilon);
}
template <typename T>
int sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

float IAlgorithm::trunc_centered_gauss(float sigma, float trunc) const
{
    // Gerando um valor gaussiano truncado
    auto ret = m_generator->random_gauss() * sigma;
    ret = (std::abs(ret) < trunc) ? ret : (sign(ret) * trunc);
    return ret;
}

const std::string& IAlgorithm::name() const
{
    return getLogName();
}


