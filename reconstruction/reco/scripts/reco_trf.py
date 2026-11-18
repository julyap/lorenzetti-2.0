#!/usr/bin/env python3
import argparse
import sys
import os

from pathlib import Path
from typing import List
from GaugiKernel import LoggingLevel, get_argparser_formatter
from GaugiKernel import ComponentAccumulator
from RootStreamBuilder import RootStreamESDReader, recordable
from CaloClusterBuilder import CaloClusterMaker
from CaloRingsBuilder import CaloRingsBuilderCfg
from EgammaBuilder import ElectronBuilderCfg
from RootStreamBuilder import RootStreamAODMaker
from RootStreamBuilder.RootStreamFlags import RootStreamAODFlags as flags

from reco.reco_job import merge_args, update_args, create_parallel_job



try:
    from MuonClusterMaker import MuonClusterMaker
    HAS_MUON_CLUSTER_MAKER = True
except ImportError:
    MuonClusterMaker = None
    HAS_MUON_CLUSTER_MAKER = False

try:
    from MuonBuilder import MuonBuilderCfg
    HAS_MUON_BUILDER = True
except ImportError:
    MuonBuilderCfg = None
    HAS_MUON_BUILDER = False


"""
Script: reco_trf.py
Purpose: Executes the offline reconstruction chain.
         Reads digitized cells (ESD), builds calorimeter clusters, computes
         ring variables (Rings), and reconstructs electron candidates.
         With --systems muon or atlas, also builds muon clusters and muons.
Usage:
    reco_trf.py -i input.ESD.root -o output.AOD.root [--systems {calorimeter,muon,atlas}]
"""
TOPOLOGY_MAP = {
    "doStdRings": ("std", "Rings"),
    "doCornerRings": ("corner", "CornerRings"),
    "doAsymRings": ("asym", "AsymRings"),
    "doStripsRings": ("strips", "StripsRings"),
    "doCrossRings": ("cross", "CrossRings"),
    "doCustomRings": ("custom", "CustomRings"),
}


def parse_args():
    """
    Parses command-line arguments for the reconstruction job.

    Returns:
        argparse.Namespace: Arguments for reconstruction configuration.
    """
    # create the top-level parser
    parser = argparse.ArgumentParser(
        description="", formatter_class=get_argparser_formatter(), add_help=False
    )

    parser.add_argument(
        "-l",
        "--output-level",
        action="store",
        dest="output_level",
        required=False,
        type=str,
        default="INFO",
        help="The output level messenger.",
    )
    parser.add_argument(
        "-c",
        "--command",
        action="store",
        dest="command",
        required=False,
        default="''",
        help="The preexec command",
    )

    # ===== SISTEMAS DE DETECÇÃO =====
    parser.add_argument('--systems', action='store',
                        dest='systems', required=False,
                        type=str, default='calorimeter',
                        choices=['calorimeter', 'muon', 'atlas'],
                        help="Which detector systems are reconstructed in this job. "
                             "'calorimeter' (default): only the calorimeter/electron chain "
                             "(the official behavior). 'muon': only the muon chain. "
                             "'atlas': both. The 'muon' and 'atlas' modes require the "
                             "MuonClusterMaker and MuonBuilder modules to be importable "
                             "(reconstruction/MuonChamber/.../python on PYTHONPATH). "
                             "This flag is orthogonal to the detector geometry options "
                             "and to the ring topology flags (RootStreamAODFlags). "
                             "It does NOT filter particles: the same ESD events are processed.")

    return merge_args(parser)


def main(
    events: List[int],
    logging_level: str,
    input_file: str | Path,
    output_file: str | Path,
    command: str,
    systems: str = 'calorimeter',
):
    """
    Main function for the reconstruction workflow.

    Orchestrates the reconstruction sequence:
    1. Reads ESD file (Cells, Particles, Seeds).
    2. Runs CaloClusterMaker to group cells into clusters.
    3. Runs CaloRingsBuilder to extract concentric ring energy sums.
    4. Runs ElectronBuilder to create electron candidates.
    5. (With --systems muon or atlas) runs MuonClusterMaker and MuonBuilder.
    6. Writes the results to an Analysis Object Data (AOD) file.

    Args:
        events (List[int]): List of event indices.
        logging_level (str): Logging verbosity.
        input_file (str | Path): Path to input ESD file.
        output_file (str | Path): Path to output AOD file.
        command (str): Optional command to execute before the sequence.
        systems (str): Which detector systems are reconstructed: 'calorimeter'
                       (default), 'muon' or 'atlas' (both).
    """
    if isinstance(input_file, Path):
        input_file = str(input_file)
    if isinstance(output_file, Path):
        output_file = str(output_file)

    # ===== SISTEMAS DE DETECÇÃO =====
    enable_calo = systems in ('calorimeter', 'atlas')
    enable_muon_system = systems in ('muon', 'atlas')
    if enable_muon_system and not (HAS_MUON_CLUSTER_MAKER and HAS_MUON_BUILDER):
        raise ImportError(
            "--systems '%s' requested, but MuonClusterMaker and/or MuonBuilder "
            "could not be imported. Make sure the corresponding Python modules "
            "are on PYTHONPATH (reconstruction/MuonChamber/...)." % systems
        )

    outputLevel = LoggingLevel.toC(logging_level)

    exec(command)

    acc = ComponentAccumulator("ComponentAccumulator", output_file)

    # ============================================
    # ESD Reader
    # ============================================
    reader_kwargs = dict(
        InputFile=input_file,
        OutputEventKey=recordable("Events"),
        OutputTruthKey=recordable("Particles"),
        OutputSeedsKey=recordable("Seeds"),
        OutputLevel=outputLevel,
        EnableCaloOutput=enable_calo,
        EnableMuonOutput=enable_muon_system,
    )
    if enable_calo:
        reader_kwargs["OutputCaloCellsKey"] = recordable("Cells")
        reader_kwargs["OutputCaloCellsTruthKey"] = recordable("TruthCells")
    if enable_muon_system:
        reader_kwargs["OutputMuonCellsKey"] = recordable("MuonCells")
        # Lista de chaves: só passa se o MuonCellBuilder tiver produzido
        # múltiplas coleções; por padrão usa a chave única acima.
        reader_kwargs["OutputMuonCellKeys"] = [recordable("MuonCells")]

    ESD = RootStreamESDReader("ESDReader", **reader_kwargs)
    ESD.merge(acc)

    # ============================================
    # Calorimeter Processing
    # ============================================
    ring_keys = []
    truth_ring_keys = []
    acc_rings = []
    acc_rings_truth = []

    if enable_calo:
        cluster = CaloClusterMaker(
            "CaloClusterMaker",
            InputCellsKey=recordable("Cells"),
            InputSeedsKey=recordable("Seeds"),
            OutputClusterKey=recordable("Clusters"),
            HistogramPath="Expert/Clusters",
            OutputLevel=outputLevel,
        )

        cluster_truth = CaloClusterMaker(
            "CaloClusterMaker_Truth",
            InputCellsKey=recordable("TruthCells"),
            InputSeedsKey=recordable("Seeds"),
            OutputClusterKey=recordable("TruthClusters"),
            HistogramPath="Expert/TruthClusters",
            OutputLevel=outputLevel,
        )

        ringsL0 = CaloRingsBuilderCfg(
            "CaloRingsBuilderL0",
            InputClusterKey=recordable("Clusters"),
            OutputRingerKey=recordable("RingsL0"),
            HistogramPath="Expert/RingsL0",
            OutputLevel=outputLevel,
            DoSigmaCut=True,
            SigmaCut=flags.SigmaCut,
        )

        hypo = ElectronBuilderCfg(
            "ElectronBuilder",
            InputClusterKey=recordable("Clusters"),
            OutputElectronKey=recordable("Electrons"),
            OutputLevel=outputLevel,
        )

        hypo_truth = ElectronBuilderCfg(
            "ElectronBuilder_Truth",
            InputClusterKey=recordable("TruthClusters"),
            OutputElectronKey=recordable("TruthElectrons"),
            OutputLevel=outputLevel,
        )

        for flag_name, (topology, key_suffix) in TOPOLOGY_MAP.items():
            if not getattr(flags, flag_name):
                continue

            extra = {}
            if topology == "corner":
                extra["CornerShift"] = flags.CornerShift
            elif topology == "cross":
                extra["CrossShift"] = flags.CrossShift
            elif topology == "custom":
                extra["RingsShiftEta"] = flags.CustomRingsShiftEta
                extra["RingsShiftPhi"] = flags.CustomRingsShiftPhi
            elif topology == "strips":
                extra["Axis"] = flags.StripsAxis

            acc_rings.append(
                CaloRingsBuilderCfg(
                    f"CaloRingsBuilder_{topology}",
                    InputClusterKey=recordable("Clusters"),
                    OutputRingerKey=recordable(key_suffix),
                    HistogramPath=f"Expert/{key_suffix}",
                    OutputLevel=outputLevel,
                    RingerTopology=topology,
                    **extra,
                )
            )

            acc_rings_truth.append(
                CaloRingsBuilderCfg(
                    f"CaloRingsBuilder_Truth_{topology}",
                    InputClusterKey=recordable("TruthClusters"),
                    OutputRingerKey=recordable(f"Truth{key_suffix}"),
                    HistogramPath=f"Expert/Truth{key_suffix}",
                    OutputLevel=outputLevel,
                    RingerTopology=topology,
                    **extra,
                )
            )

            ring_keys.append(recordable(key_suffix))
            truth_ring_keys.append(recordable(f"Truth{key_suffix}"))

        # Sequência do calorímetro
        acc += cluster
        for rings in acc_rings:
            acc += rings
        acc += ringsL0
        acc += hypo
        acc += cluster_truth
        for rings_truth in acc_rings_truth:
            acc += rings_truth
        acc += hypo_truth

    # ============================================
    # Muon Processing
    # ============================================
    if enable_muon_system:
        muon_cluster = MuonClusterMaker(
            name="MuonClusterMaker",
            InputCellsKey=recordable("MuonCells"),
            InputSeedsKey=recordable("Seeds"),
            OutputClusterKey=recordable("MuonClusters"),
            EtaWindow=0.3,
            PhiWindow=0.3,
            XWindow=1500.0,
            YWindow=1500.0,
            ZWindow=1500.0,
            MinCenterEnergy=0.01,
            DoForwardMoments=False,
            HistPath="Expert/MuonClusters",
            OutputLevel=outputLevel,
        )
        acc += muon_cluster

        muon_builder = MuonBuilderCfg(
            "MuonBuilder",
            InputMuonClusterKey=recordable("MuonClusters"),
            OutputMuonKey=recordable("Muons"),
            OutputLevel=outputLevel,
        )
        acc += muon_builder

    # ============================================
    # AOD Writer
    # ============================================
    aod_kwargs = dict(
        InputEventKey=recordable("Events"),
        InputSeedsKey=recordable("Seeds"),
        InputTruthKey=recordable("Particles"),
        OutputEventKey=recordable("Events"),
        OutputSeedsKey=recordable("Seeds"),
        OutputTruthKey=recordable("Particles"),
        OutputLevel=outputLevel,
        EnableCaloOutput=enable_calo,
        EnableMuonOutput=enable_muon_system,
        DumpCells=False,
        DumpMuonCells=False,
        DumpMuonClusters=False,
    )

    if enable_calo:
        aod_kwargs.update(
            InputCellsKey=recordable("Cells"),
            InputTruthCellsKey=recordable("TruthCells"),
            InputClusterKey=recordable("Clusters"),
            InputTruthClusterKey=recordable("TruthClusters"),
            InputRingerKeys=ring_keys,
            InputTruthRingerKeys=truth_ring_keys,
            InputElectronKey=recordable("Electrons"),
            InputTruthElectronKey=recordable("TruthElectrons"),
            InputRingerL0Key=recordable("RingsL0"),
        )

    if enable_muon_system:
        aod_kwargs.update(
            InputMuonCellsKey=recordable("MuonCells"),
            InputMuonClusterKey=recordable("MuonClusters"),
            InputMuonKey=recordable("Muons"),
            OutputMuonCellsKey=recordable("MuonCells"),
            OutputMuonClusterKey=recordable("MuonClusters"),
            OutputMuonKey=recordable("Muons"),
        )

    AOD = RootStreamAODMaker("RootStreamAODMaker", **aod_kwargs)
    acc += AOD

    acc.run(events)


if __name__ == "__main__":
    parser = parse_args()
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()

    # ===== SISTEMAS: validação cruzada =====
    if args.systems in ('muon', 'atlas'):
        if not HAS_MUON_CLUSTER_MAKER:
            parser.error("--systems %s requires the MuonClusterMaker module "
                         "(reconstruction/MuonChamber/MuonClusterBuilder/python on PYTHONPATH)" % args.systems)
        if not HAS_MUON_BUILDER:
            parser.error("--systems %s requires the MuonBuilder module "
                         "(reconstruction/MuonChamber/MuonBuilder/python on PYTHONPATH)" % args.systems)

    args = update_args(args)
    pool = create_parallel_job(args)
    pool(
        main,
        logging_level=args.output_level,
        command=args.command,
        systems=args.systems,
    )
