import ROOT
import numpy as np
import glob
import matplotlib.pyplot as plt
from math import sqrt
import os
from collections.abc import Iterable

def calculate_invariant_mass(e, p):
    px_e = e[0] * np.cos(e[2])
    py_e = e[0] * np.sin(e[2])
    pz_e = e[0] * np.sinh(e[1])
    e_e = e[0] * np.cosh(e[1])

    px_p = p[0] * np.cos(p[2])
    py_p = p[0] * np.sin(p[2])
    pz_p = p[0] * np.sinh(p[1])
    e_p = p[0] * np.cosh(p[1])

    return sqrt((e_e + e_p)**2 - (px_e + px_p)**2 - (py_e + py_p)**2 - (pz_e + pz_p)**2) / 1000

def is_iterable_but_not_str(obj):
    return isinstance(obj, Iterable) and not isinstance(obj, (str, bytes))

# Caminho dos arquivos
input_dir = "/home/administrador/apptainer-1.3.0/lorenzetti/EVT/output"
root_files = glob.glob(f"{input_dir}/job_*/step_5/Zee.*.root")

invariant_masses = []
eventos_validos = 0

for filepath in root_files[:1]:  # Processa só o primeiro arquivo para teste
    try:
        file = ROOT.TFile.Open(filepath)
        tree = file.Get("physics")

        for i, event in enumerate(tree):
            if not all(hasattr(event, attr) for attr in ['mc_et', 'mc_eta', 'mc_phi', 'mc_pdgid']):
                continue

            mc_et = event.mc_et
            mc_eta = event.mc_eta
            mc_phi = event.mc_phi
            mc_pdgid = event.mc_pdgid

            if i == 0:
                print("🧪 Tipos detectados no primeiro evento:")
                print(f"mc_et: {type(mc_et)}, mc_eta: {type(mc_eta)}, mc_phi: {type(mc_phi)}, mc_pdgid: {type(mc_pdgid)}")

            if i < 3:
                print(f"🔍 PDG IDs do evento {i}: {[int(pdgid) for pdgid in mc_pdgid]}")

            if not all(is_iterable_but_not_str(x) for x in [mc_et, mc_eta, mc_phi, mc_pdgid]):
                continue

            if not (len(mc_et) == len(mc_eta) == len(mc_phi) == len(mc_pdgid)):
                continue

            pdgids = [int(pid) for pid in mc_pdgid]
            if 11 not in pdgids or -11 not in pdgids:
                continue

            eventos_validos += 1

            particles = []
            for et, eta, phi, pdgid in zip(mc_et, mc_eta, mc_phi, mc_pdgid):
                if abs(pdgid) == 11:
                    particles.append((et, eta, phi, pdgid))

            electrons = [p[:3] for p in particles if p[3] == -11]
            positrons = [p[:3] for p in particles if p[3] == 11]

            for e in electrons:
                for p in positrons:
                    invariant_masses.append(calculate_invariant_mass(e, p))

    except Exception as e:
        print(f"Erro ao processar {filepath}: {str(e)}")

# Plotagem
if invariant_masses:
    plt.figure(figsize=(10, 6))
    plt.hist(invariant_masses, bins=100, range=(70, 110), color='darkblue', alpha=0.7)
    plt.xlabel("Massa Invariante [GeV]")
    plt.ylabel("Número de Eventos")
    plt.title("Distribuição de Massa Invariante e⁺e⁻ (usando MC truth)")
    plt.axvline(91.1876, color='red', linestyle='--', label='Massa do Z')
    plt.legend()
    plt.grid(True)

    output_dir = "histogramas"
    os.makedirs(output_dir, exist_ok=True)
    output_path = f"{output_dir}/massa_invariante_mc.png"
    plt.savefig(output_path)
    plt.close()
    print(f"✅ Histograma salvo em: {output_path}")
    print(f"Total de massas calculadas: {len(invariant_masses)}")
    print(f"🔢 Total de eventos com e⁺e⁻: {eventos_validos}")
else:
    print("⚠️ Nenhuma massa invariante calculada. Verifique:")
    print("- Se mc_et, mc_eta, mc_phi, mc_pdgid existem e são vetores")
    print("- Se há pares e⁺e⁻ no evento")

