#include <cmath>
#include "Upsilon.h"
#include "helper.h"

using namespace generator;

Upsilon::Upsilon(const std::string& name, IGenerator* gen)
  : IMsgService(name),  // Inicializar IMsgService primeiro
    IAlgorithm(gen)     // Inicializar IAlgorithm depois
{
  declareProperty("EtaMax", m_etaMax = 1.4);
  declareProperty("MinPt", m_minPt = 0.0);
  declareProperty("ZeroVertexParticles", m_zeroVertexParticles = false);
  declareProperty("ForceForwardMuon", m_forceForwardUpsilon = false);
}

Upsilon::~Upsilon() {}

StatusCode Upsilon::initialize()
{
  setMsgLevel(m_outputLevel);
  if (generator()->initialize().isFailure()) {
    MSG_FATAL("Unable to initialize the Upsilon generator. Abort!");
  }

  return StatusCode::SUCCESS;
}

StatusCode Upsilon::execute(generator::Event& ctx)
{
  HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::MM);

  if (generator()->execute(evt).isFailure()) {
    return StatusCode::FAILURE;
  }

  const auto main_event_t = sample_t();
  const auto main_event_z = sample_z();

  std::vector<const HepMC3::GenParticle*> upsilon;

  for (auto part : evt.particles()) {
    if (part->abs_pid() == 13 && ParticleHelper::isFinal(part.get())) {
      if (part->parents().empty()) {
        continue;
      }

      auto mother = part->parents().at(0);
      if (mother->pid() == 553) {  // PDG ID do Upsilon
        float eta = part->momentum().eta();
        float pt = part->momentum().pt();
        if (std::abs(eta) < m_etaMax && pt > (m_minPt / 1.e3)) {
          upsilon.push_back(part.get());
        }
      }
    }
  }

  if (upsilon.empty()) {
    MSG_DEBUG("No Upsilon event inside this event");
    throw NotInterestingEvent();
  }

  if (m_forceForwardUpsilon) {
    int fwdMuonCounter = 0;
    for (auto& mu : upsilon) {
      float eta = mu->momentum().eta();
      if (std::abs(eta) > 2.5 && std::abs(eta) < 3.2) {
        fwdMuonCounter++;
      }
    }
    if (fwdMuonCounter == 0 || fwdMuonCounter > 1) {
      MSG_INFO("No forward muons or two forward muons");
      throw NotInterestingEvent();
    }
  }

  MSG_INFO("Filling Upsilon events into the context...");

  for (auto& mu : upsilon) {
    auto seed = generator::Seed(mu->momentum().eta(), mu->momentum().phi());

    const auto vtx = mu->production_vertex();
    float zVertexPos = vtx->position().pz() + main_event_z;

    if (m_zeroVertexParticles) {
      zVertexPos = vtx->position().pz();
    }

    seed.emplace_back(1, 0,
                      mu->pid(),
                      mu->momentum().px(),
                      mu->momentum().py(),
                      mu->momentum().pz(),
                      mu->momentum().eta(),
                      mu->momentum().phi(),
                      vtx->position().px(),
                      vtx->position().py(),
                      zVertexPos,
                      vtx->position().t() + main_event_t,
                      mu->momentum().e(),
                      mu->momentum().pt());
    MSG_INFO("Add particle with PDGID " << mu->pid() << " and vertex x=" << vtx->position().px()
                                       << ", y=" << vtx->position().py()
                                       << ", z=" << zVertexPos
                                       << ", t=" << vtx->position().t() + main_event_t
                                       << " into the context.");
    ctx.push_back(seed);
  }

  ctx.setEventNumber(evt.event_number());
  return StatusCode::SUCCESS;
}

StatusCode Upsilon::finalize()
{
  MSG_INFO("Finalize the Upsilon Event.");
  if (generator()->finalize().isFailure()) {
    MSG_FATAL("Unable to finalize the Upsilon generator. Abort!");
  }
  return StatusCode::SUCCESS;
}

