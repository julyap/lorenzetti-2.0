# ============================================================================
# Arquivo: generate_pulse.py
# Autora: Julya Pacheco Ferreira
# Descrição: Gera as formas de onda (pulse shapes) das câmaras de múon do
#            ATLAS (MDT, CSC, TGC e RPC) e salva cada uma em um arquivo .dat
#            no formato "time amplitude" (duas colunas), que é lido pelo
#            MuonPulseGenerator. Os pulsos são parametrizados com base nos
#            dados dos TDRs das câmaras, com diferentes resoluções temporais
#            e constantes de rise/decay para cada tecnologia. O script também
#            imprime um resumo do número de amostras e do tamanho dos
#            arquivos gerados.
# ============================================================================

import numpy as np


# ============================================================================
# Geradores de pulso por tipo de câmara
# ============================================================================

def generate_mdt_pulse():
    """
    Gera o pulso do MDT (Monitored Drift Tube) na faixa [-100, 2000] ns
    com passo de 10 ns. Modelo simplificado: rise exponencial até 300 ns
    e decay exponencial a partir de 300 ns. Adiciona ruído gaussiano de
    2% quando a amplitude é significativa.
    """

    # Grade temporal: de -100 ns até 2000 ns, passo de 10 ns
    times = np.arange(-100, 2001, 10)
    pulses = []

    for t in times:
        # Antes do tempo zero, sem sinal
        if t < 0:
            amp = 0.0
        # Fase de subida (rise) até 300 ns
        elif t < 300:
            amp = 1.0 - np.exp(-t/80.0)
        # Fase de decaimento (decay) a partir de 300 ns
        else:
            amp = np.exp(-(t-300)/350.0)

        # Adiciona ruído gaussiano de 2% quando a amplitude é significativa
        if amp > 0.01:
            amp *= (1.0 + 0.02 * np.random.randn())
        )

    return pulses


def generate_csc_pulse():
    """
    Gera o pulso do CSC (Cathode Strip Chamber) na faixa [-50, 200] ns
    com passo de 2 ns. Rise até 16 ns e decay a partir daí, com
    constantes de tempo mais rápidas que as do MDT.
    """

    # Grade temporal: de -50 ns até 200 ns, passo de 2 ns
    times = np.arange(-50, 201, 2)
    pulses = []

    for t in times:
        # Antes do tempo zero, sem sinal
        if t < 0:
            amp = 0.0
        # Fase de subida (rise) até 16 ns
        elif t < 16:
            amp = 1.0 - np.exp(-t/4.0)
        # Fase de decaimento (decay) a partir de 16 ns
        else:
            amp = np.exp(-(t-16)/15.0)



    return pulses


def generate_tgc_pulse():
    """
    Gera o pulso do TGC (Thin Gap Chamber). Usa a mesma resolução do RPC
    (0.2 ns) e o mesmo range [-10, 20] ns, para compatibilidade com o
    formato lido pelo MuonPulseGenerator. Rise até 3.5 ns e decay a
    partir daí. Adiciona ruído gaussiano de 2% quando a amplitude é
    significativa.
    """

    # MUDEI AQUI: mesma resolução do RPC (0.2 ns) e mesmo range [-10, 20]
    times = np.arange(-10, 20.1, 0.2)  # ← IGUAL AO RPC!
    pulses = []

    for t in times:
        # Antes do tempo zero, sem sinal
        if t < 0:
            amp = 0.0
        # Fase de subida até 3.5 ns (pico do TGC)
        elif t < 3.5:  # Pico do TGC é em 3.5 ns
            amp = 1.0 - np.exp(-t/1.0)  # Rise time mais rápido que RPC
        # Fase de decaimento a partir de 3.5 ns
        else:
            amp = np.exp(-(t-3.5)/5.0)   # Decay mais lento que RPC

        # Adiciona um pouco de ruído
        if amp > 0.01:
            amp *= (1.0 + 0.02 * np.random.randn())



    # Log de configuração do pulso gerado
    print(f"TGC pulse generated: {len(times)} samples")
    print(f"  Range: [{times[0]}, {times[-1]}] ns")
    print(f"  Step: 0.2 ns")
    print(f"  Peak at 3.5 ns, amplitude 1.0")
    return pulses


def generate_rpc_pulse():
    """
    Gera o pulso do RPC (Resistive Plate Chamber) na faixa [-10, 21] ns
    com passo de 0.2 ns. Rise até 1.5 ns e decay a partir daí.
    """

    # Grade temporal: de -10 ns até 21 ns, passo de 0.2 ns
    times = np.arange(-10, 21, 0.2)
    pulses = []

    for t in times:
        # Antes do tempo zero, sem sinal
        if t < 0:
            amp = 0.0
        # Fase de subida (rise) até 1.5 ns
        elif t < 1.5:
            amp = 1.0 - np.exp(-t/0.5)
        # Fase de decaimento (decay) a partir de 1.5 ns
        else:
            amp = np.exp(-(t-1.5)/1.2)



    return pulses


# ============================================================================
# Função auxiliar para salvar o pulso em arquivo .dat
# ============================================================================

def save_pulse(filename, header, pulses):
    """
    Salva a lista de amostras (cada uma no formato "time amplitude") em
    um arquivo .dat, com um cabeçalho de comentário. Também imprime as
    primeiras e as últimas 10 linhas para debug.
    """
    with open(filename, 'w') as f:
        f.write(header + '\n')
        for i, pulse in enumerate(pulses):
            f.write(pulse + '\n')
            # Debug: mostra as primeiras 10 e últimas 10 linhas
            if i < 10 or i > len(pulses)-11:
                print(f"  {pulse}")


# ============================================================================
# Geração e salvamento dos pulsos
# ============================================================================

# Generate all pulses
print("=" * 60)
print("Generating ATLAS Muon Pulse Shapes")
print("=" * 60)

# ---------------------------------------------------------------------------
# MDT
# ---------------------------------------------------------------------------
print("\n1. Generating MDT pulse...")
mdt_pulses = generate_mdt_pulse()
save_pulse('pulseMDT.dat',
           '# ATLAS MDT Pulse Shape - TDR 1997\n# Gas: Ar-N2-CH4 (91/4/5%), 3bar, 3270V\n# Max drift: 480ns, Resolution: 80um',
           mdt_pulses)
print(f"   Saved {len(mdt_pulses)} samples to pulseMDT.dat")

# ---------------------------------------------------------------------------
# CSC
# ---------------------------------------------------------------------------
print("\n2. Generating CSC pulse...")
csc_pulses = generate_csc_pulse()
save_pulse('pulseCSC.dat',
           '# ATLAS CSC Pulse Shape - TDR 1997\n# Gas: Ar-CO2-CF4 (30/50/20%), 2600V\n# Cathode spacing: 5.08mm, Wire pitch: 2.54mm',
           csc_pulses)
print(f"   Saved {len(csc_pulses)} samples to pulseCSC.dat")

# ---------------------------------------------------------------------------
# TGC
# ---------------------------------------------------------------------------
print("\n3. Generating TGC pulse (RPC-compatible)...")
tgc_pulses = generate_tgc_pulse()
save_pulse('pulseTGC.dat',
           '# ATLAS TGC Pulse Shape - TDR 1997\n# Gas: CO2/n-C5H12 (55/45%), 3100V\n# Gap: 2.8mm, Gain: 1e6, >99% eff @25ns\n# MODIFIED: RPC-compatible format (0.2ns step)',
           tgc_pulses)
print(f"   Saved {len(tgc_pulses)} samples to pulseTGC.dat")

# ---------------------------------------------------------------------------
# RPC
# ---------------------------------------------------------------------------
print("\n4. Generating RPC pulse...")
rpc_pulses = generate_rpc_pulse()
save_pulse('pulseRPC.dat',
           '# ATLAS RPC Pulse Shape - TDR 1996\n# Gas: C2H2F4-C4H10 (97/3%), 8900V\n# Gap: 2.0mm, Time res: 1.5ns RMS',
           rpc_pulses)
print(f"   Saved {len(rpc_pulses)} samples to pulseRPC.dat")

print("\n" + "=" * 60)
print("Pulse generation complete!")
print("=" * 60)

# ---------------------------------------------------------------------------
# Verificação final: tamanho e número de linhas de cada arquivo
# ---------------------------------------------------------------------------
print("\nFile sizes:")
import os
for filename in ['pulseMDT.dat', 'pulseCSC.dat', 'pulseTGC.dat', 'pulseRPC.dat']:
    if os.path.exists(filename):
        size = os.path.getsize(filename)
        with open(filename, 'r') as f:
            lines = sum(1 for line in f)
        print(f"  {filename}: {size} bytes, {lines} lines")
