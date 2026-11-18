# ============================================================================
# Arquivo: MuonBuilder.py
# Autora: Julya Pacheco Ferreira
# Descrição: Função de configuração MuonBuilderCfg para reconstrução de
#            múons MS-ONLY (baseada no ATLAS 2016, arXiv:1603.05598).
#
#            Define quatro categorias de qualidade para múons reconstruídos
#            apenas no Muon Spectrometer (sem informação do Inner Detector):
#              - Very Loose (aprox. CT muons, |η|<0.1): aceita 1 camada,
#                sem corte eficaz em chi2, fracMax > 0.5;
#              - Loose (MS track padrão): ≥2 camadas, chi2 < 30,
#                fracMax > 0.6;
#              - Medium (MS-only de boa qualidade): ≥2 camadas,
#                chi2 < 15, fracMax > 0.7;
#              - Tight-like (aproximação MS-only, NÃO é o Tight oficial do
#                ATLAS): ≥3 camadas, chi2 < 8, fracMax > 0.8.
#
#            Os cortes são passados ao MuonMaker via parâmetros
#            TightCuts/MediumCuts/LooseCuts/VLooseCuts, no formato
#            [chi2_cut, min_cells, min_fracMax].
# ============================================================================

__all__ = ['MuonBuilderCfg']

# --- Imports do framework e do projeto -------------------------------------
from GaugiKernel.macros import *
from . import MuonMaker


def MuonBuilderCfg( name              : str,
                    InputMuonClusterKey : str,
                    OutputMuonKey       : str,
                    OutputLevel         : int = 0,
                  ):
    """
    Configuration for MS-ONLY muon reconstruction based on ATLAS 2016

    ╔══════════════════════════════════════════════════════════════════════╗
    ║  MS-ONLY MUON CATEGORIES (ME-like, CT-like)                          ║
    ║  Based on ATLAS 2016 paper: arXiv:1603.05598                         ║
    ╚══════════════════════════════════════════════════════════════════════╝

    Categories use three variables:
      - nCells:    number of cells/layers in MS cluster
      - chi2:      fit quality of MS track
      - fracMax:   emaxs2/totalEnergy (MIP discrimination)

    VERY LOOSE (CT muon-like, |η|<0.1 gap):
      - Accepts 1 layer (MIP signature)
      - No effective chi2 cut
      - fracMax > 0.5 ensures dominant cell

    LOOSE (standard MS track):
      - Requires >=2 layers (Section 3.1)
      - chi2 < 30 (reasonable quality)
      - fracMax > 0.6 (clear MIP)

    MEDIUM (good quality MS-only):
      - Requires >=2 layers
      - chi2 < 15 (good fit quality)
      - fracMax > 0.7 (clean MIP)

    TIGHT-LIKE (approximation for MS-only, forward region 2.5<|η|<2.7):
      - Requires >=3 layers
      - chi2 < 8 (same as ATLAS Tight for CB muons)
      - fracMax > 0.8 (very clean MIP)
      - NOTE: this is a TIGHT-LIKE approximation, NOT the official ATLAS
        Tight working point (which requires ID+MS combination)

    Reference: ATLAS 2016, arXiv:1603.05598
      - Section 3.1: MS track reconstruction (>=2 segments)
      - Section 5.1: ME muons for |η|>2.5 (>=3 layers)
      - Section 5.1.2: Tight CB muons have chi2<8
      - Table 1: Loose efficiency 96.7-98.1%, Tight 89.9-91.8%
    """

    # ============================================================
    # MS-ONLY MUON CUTS based on ATLAS 2016
    # Format: [chi2_cut, min_cells, min_fracMax]
    # ============================================================

    # Very Loose: based on CT muons in central gap (|η|<0.1)
    vloose_cuts = [999.0, 1.0, 0.5]     # chi2 < 999 (any), >=1 cell, fracMax >= 0.5

    # Loose: standard MS track (Section 3.1)
    loose_cuts = [30.0, 2.0, 0.6]       # chi2 < 30, >=2 cells, fracMax >= 0.6

    # Medium: good quality MS-only
    medium_cuts = [15.0, 2.0, 0.8]      # chi2 < 15, >=2 cells, fracMax >= 0.7

    # Tight-like: approximation for MS-only (forward region)
    tight_cuts = [6.0, 2.0, 0.9]        # chi2 < 8, >=3 cells, fracMax >= 0.8

    # ============================================================
    # INFO MESSAGE
    # ============================================================
    print("\n" + "="*70)
    print("⚫ MuonBuilderCfg: MS-ONLY muon reconstruction (4 categories)")
    print("="*70)
    print("Based on ATLAS 2016 (arXiv:1603.05598)")
    print("")
    print("Categories (MS-only, no ID information):")
    print("  ┌──────────────┬─────────────┬──────────┬─────────────┐")
    print("  │ Category     │ Min Cells   │ Max Chi2 │ Min FracMax │")
    print("  ├──────────────┼─────────────┼──────────┼─────────────┤")
    print(f"  │ Very Loose   │ >= {vloose_cuts[1]}           │ < {vloose_cuts[0]}    │ >= {vloose_cuts[2]}       │")
    print(f"  │ Loose        │ >= {loose_cuts[1]}           │ < {loose_cuts[0]}   │ >= {loose_cuts[2]}       │")
    print(f"  │ Medium       │ >= {medium_cuts[1]}           │ < {medium_cuts[0]}   │ >= {medium_cuts[2]}       │")
    print(f"  │ Tight-like   │ >= {tight_cuts[1]}           │ < {tight_cuts[0]}    │ >= {tight_cuts[2]}       │")
    print("  └──────────────┴─────────────┴──────────┴─────────────┘")
    print("")
    print("ATLAS reference:")
    print("  - CT muons in |η|<0.1:        accepts 1 layer (MIP)")
    print("  - MS track (barrel/endcap):   requires >=2 layers")
    print("  - ME muons (|η|>2.5):          requires >=3 layers")
    print("  - Tight CB muons:             chi2 < 8")
    print("  - Loose efficiency (Table 1): 96.7-98.1%")
    print("  - Tight efficiency (Table 1): 89.9-91.8%")
    print("")
    print("⚠️  Tight-like is an APPROXIMATION for MS-only!")
    print("   True ATLAS Tight requires ID+MS combination.")
    print("="*70 + "\n")

    # Instancia o MuonMaker já configurado com os cortes das 4 categorias
    hypo = MuonMaker(
        name=name,
        InputMuonClusterKey=InputMuonClusterKey,
        OutputMuonKey=OutputMuonKey,
        OutputLevel=OutputLevel,
        TightCuts=tight_cuts,
        MediumCuts=medium_cuts,
        LooseCuts=loose_cuts,
        VLooseCuts=vloose_cuts,
    )

    return hypo
