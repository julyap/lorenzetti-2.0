import ROOT
import os
import glob
import itertools
import numpy as np
import matplotlib.pyplot as plt

ROOT.gROOT.SetBatch(True)  # Evita abrir janelas gráficas do ROOT

# Caminho base
input_dir = "/home/administrador/apptainer-1.3.0/lorenzetti/EVT/output_jpsi"

# Coletar todos os arquivos ROOT
root_files = glob.glob(f"{input_dir}/job_*/step_5/Zee.*.root")
print(f"Encontrados {len(root_files)} arquivos .root")

# Nomes dos branches que estamos interessados
branches = [
    'EventNumber', 'RunNumber', 'avgmu', 'cl_eta', 'cl_phi', 'cl_e', 'cl_et',
    'cl_deta', 'cl_dphi', 'cl_e0', 'cl_e1', 'cl_e2', 'cl_e3', 'cl_ehad1', 'cl_ehad2',
    'cl_ehad3', 'cl_etot', 'cl_e233', 'cl_e237', 'cl_e277', 'cl_emaxs1', 'cl_emaxs2',
    'cl_e2tsts1', 'cl_reta', 'cl_rphi', 'cl_rhad', 'cl_rhad1', 'cl_eratio', 'cl_f0',
    'cl_f1', 'cl_f2', 'cl_f3', 'cl_weta2', 'cl_rings', 'cl_secondR', 'cl_lambdaCenter',
    'cl_secondLambda', 'cl_fracMax', 'cl_lateralMom', 'cl_longitudinalMom', 'el_eta', 
    'el_et', 'el_phi', 'el_tight', 'el_medium', 'el_loose', 'el_vloose', 'seed_eta', 
    'seed_phi', 'mc_pdgid', 'mc_eta', 'mc_phi', 'mc_e', 'mc_et'
]

# Dicionário para armazenar dados
data = {branch: [] for branch in branches}

# Ler arquivos ROOT
for filepath in root_files:
    print(f"Processando: {filepath}")
    file = ROOT.TFile.Open(filepath)
    if not file or file.IsZombie():
        print(f"[WARN] Falha ao abrir arquivo: {filepath}")
        continue

    tree = file.Get("physics")
    if not tree:
        print(f"[WARN] Árvore 'physics' não encontrada em {filepath}")
        continue

    # Coletar dados de cada branch para cada evento
    for event in tree:
        for branch in branches:
            value = getattr(event, branch, None)
            if value is not None:
                data[branch].append(value)



# Criar diretório para salvar histogramas
output_dir = "histogramas"
os.makedirs(output_dir, exist_ok=True)

# Gerar histogramas e salvar em arquivos separados
for branch, values in data.items():
    if len(values) == 0:
        print(f"[WARN] Branch '{branch}' vazio, pulando.")
        continue

    try:
        # Se os valores forem listas/arrays dentro da lista, achatar para 1D
        if all(isinstance(v, (list, tuple, np.ndarray)) for v in values):
            flat_values = list(itertools.chain.from_iterable(values))
        else:
            flat_values = values

        # Converter para array numpy float para evitar erros com bool
        numeric_values = np.array(flat_values, dtype=float)

        if numeric_values.size == 0:
            print(f"[WARN] Dados vazios para branch '{branch}', pulando.")
            continue

        plt.figure()
        plt.hist(numeric_values, bins=20, color='blue', alpha=0.7)
        plt.title(branch)
        plt.xlabel(branch)
        plt.ylabel("Eventos")
        plt.tight_layout()
        plt.savefig(f"{output_dir}/{branch}.png")
        plt.close()
        print(f"✅ Histograma salvo: {output_dir}/{branch}.png")

    except Exception as e:
        print(f"[ERRO] Falha ao gerar histograma para '{branch}': {e}")
