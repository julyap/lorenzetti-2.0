import ROOT
import os
import glob
import numpy as np
import matplotlib.pyplot as plt
from tqdm import tqdm  # Para barra de progresso

ROOT.gROOT.SetBatch(True)
ROOT.gROOT.SetMacroPath(".:/usr/local/lib/root")  # Ajuste para seu sistema

# Configuração de diretórios
input_dir = "/home/administrador/apptainer-1.3.0/lorenzetti/EVT/output"
output_dir = "pdgid_analysis"
os.makedirs(output_dir, exist_ok=True)

# Coletar arquivos (limitando para teste)
root_files = glob.glob(f"{input_dir}/job_*/step_5/Zee.*.root")  # Apenas 5 arquivos para teste
print(f"Processando {len(root_files)} arquivos...")

# Estrutura para armazenar PDG IDs
pdg_ids = []

# Função segura para extração
def get_pdg_safe(tree):
    temp_ids = []
    for event in tree:
        try:
            ids = event.mc_pdgid
            if hasattr(ids, '__len__'):  # Se for vetor/lista
                temp_ids.extend(list(ids))
            else:  # Valor único
                temp_ids.append(ids)
        except:
            continue
    return temp_ids

# Processamento principal
for filepath in tqdm(root_files, desc="Arquivos ROOT"):
    try:
        f = ROOT.TFile(filepath)
        if not f or f.IsZombie():
            continue
            
        tree = f.Get("physics")
        if not tree:
            continue
            
        pdg_ids.extend(get_pdg_safe(tree))
        f.Close()
        
    except Exception as e:
        print(f"\nErro no arquivo {filepath}: {str(e)}")
        continue

# Análise dos resultados
if not pdg_ids:
    print("Nenhum PDG ID encontrado!")
    exit()

print(f"\nTotal de PDG IDs coletados: {len(pdg_ids)}")
print("Primeiros 10 valores:", pdg_ids[:10])

# Converter para array numpy
pdg_array = np.array(pdg_ids, dtype=int)

# Análise estatística
unique, counts = np.unique(pdg_array, return_counts=True)
total = sum(counts)

print("\nDistribuição de PDG IDs:")
for id, count in zip(unique, counts):
    print(f"ID: {id:6d} | Count: {count:6d} | Frac: {count/total:.2%}")

# Plot
plt.figure(figsize=(12,6))
plt.bar(unique.astype(str), counts, color='darkblue')
plt.title("Distribuição de Partículas (PDG ID)")
plt.xlabel("PDG ID")
plt.ylabel("Contagem")
plt.grid(axis='y', alpha=0.3)
plt.xticks(rotation=45)
plt.tight_layout()
plt.savefig(f"{output_dir}/pdg_distribution.png")
print(f"\nGráfico salvo em {output_dir}/pdg_distribution.png")
