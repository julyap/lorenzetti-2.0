#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path
from typing import List

import ROOT as _ROOT
from expand_folders import expand_folders
from GaugiKernel import LoggingLevel, get_argparser_formatter, ComponentAccumulator
from RootStreamBuilder import RootStreamAODReader, recordable
from RootStreamBuilder import RootStreamNtupleMaker
from reco.reco_job import merge_args, update_args, create_parallel_job


def _detect_ring_keys(input_file: str) -> tuple[list, list]:
    """Scan AOD CollectionTree to find every CaloRingsContainer_* branch."""
    f = _ROOT.TFile.Open(input_file)
    if not f or f.IsZombie():
        return ["Rings"], ["TruthRings"]
    tree = f.Get("CollectionTree")
    if not tree:
        f.Close()
        return ["Rings"], ["TruthRings"]

    ring_keys, truth_ring_keys = [], []
    for branch in tree.GetListOfBranches():
        name = branch.GetName()
        if not name.startswith("CaloRingsContainer_"):
            continue
        key = name[len("CaloRingsContainer_") :]
        if key == "RingsL0":
            continue
        if key.startswith("Truth"):
            truth_ring_keys.append(key)
        else:
            ring_keys.append(key)

    f.Close()
    return ring_keys or ["Rings"], truth_ring_keys or ["TruthRings"]


def _detect_muon_branches(input_file: str) -> tuple[bool, bool]:
    f = _ROOT.TFile.Open(input_file)
    if not f or f.IsZombie():
        return False, False
    tree = f.Get("CollectionTree")
    if not tree:
        f.Close()
        return False, False

    has_muons = False
    has_muon_clusters = False
    for branch in tree.GetListOfBranches():
        name = branch.GetName()
        if name.startswith("MuonContainer_"):
            has_muons = True
        elif name.startswith("MuonClusterContainer_"):
            has_muon_clusters = True

    f.Close()
    return has_muons, has_muon_clusters


def parse_args():
    parser = argparse.ArgumentParser(
        description="", formatter_class=get_argparser_formatter(), add_help=False
    )
    parser.add_argument("-l", "--output-level", dest="output_level",
                        default="INFO", help="Logging verbosity.")
    parser.add_argument('--systems', action='store',
                        dest='systems', required=False,
                        type=str, default='calorimeter',
                        choices=['calorimeter', 'muon', 'atlas'],
                        help="Which detector systems are present in the input AOD "
                             "and should be dumped into the flat ntuple. "
                             "'calorimeter' (default): only calorimeter/electron branches. "
                             "'muon': standalone muon ntuple (one entry per event; the AOD "
                             "must have been produced with reco_trf.py --systems muon). "
                             "'atlas': both (one entry per electron, with mu_* filled).")
    return merge_args(parser)


def main(events: List[int],
         logging_level: str,
         input_file: str | Path,
         output_file: str | Path,
         systems: str = 'calorimeter'):
    if isinstance(input_file, Path):
        input_file = str(input_file)
    if isinstance(output_file, Path):
        output_file = str(output_file)

    enable_calo = systems in ('calorimeter', 'atlas')
    enable_muon_system = systems in ('muon', 'atlas')

    outputLevel = LoggingLevel.toC(logging_level)

    ring_keys, truth_ring_keys = _detect_ring_keys(input_file)
    has_muons, has_muon_clusters = _detect_muon_branches(input_file)

    if enable_muon_system and not (has_muons or has_muon_clusters):
        print(f"⚠️  --systems {systems}: no MuonContainer_*/MuonClusterContainer_* "
              f"branches found in {input_file}. The muon branches in the ntuple will be empty.")

    acc = ComponentAccumulator("ComponentAccumulator", output_file)

    # ===== AOD READER =====
    aod_kwargs = dict(
        InputFile=input_file,
        OutputLevel=outputLevel,
        OutputEventKey=recordable("Events"),
        OutputTruthKey=recordable("Particles"),
        OutputSeedsKey=recordable("Seeds"),
        NtupleName="CollectionTree",
        EnableCaloOutput=enable_calo,
        EnableMuonOutput=enable_muon_system,
    )
    if enable_calo:
        aod_kwargs.update(
            OutputClusterKey=recordable("Clusters"),
            OutputRingerKeys=[recordable(k) for k in ring_keys],
            OutputElectronKey=recordable("Electrons"),
            OutputTruthClusterKey=recordable("TruthClusters"),
            OutputTruthRingerKeys=[recordable(k) for k in truth_ring_keys],
            OutputRingerL0Key=recordable("RingsL0"),
            OutputTruthElectronKey=recordable("TruthElectrons"),
        )
    if enable_muon_system:
        aod_kwargs.update(
            OutputMuonKey=recordable("Muons"),
            OutputMuonClusterKey=recordable("MuonClusters"),
        )
    aod = RootStreamAODReader("AODReader", **aod_kwargs)
    aod.merge(acc)

    # ===== NTUPLE MAKER =====
    ntuple_kwargs = dict(
        OutputLevel=outputLevel,
        InputEventKey=recordable("Events"),
        InputTruthKey=recordable("Particles"),
        OutputNtupleName="physics",
        EnableCaloOutput=enable_calo,
        EnableMuonOutput=enable_muon_system,
    )
    if enable_calo:
        ntuple_kwargs.update(
            InputClusterKey=recordable("Clusters"),
            InputRingerKeys=[recordable(k) for k in ring_keys],
            InputRingerL0Key=recordable("RingsL0"),
            InputElectronKey=recordable("Electrons"),
            InputSeedsKey=recordable("Seeds"),
            InputTruthClusterKey=recordable("TruthClusters"),
            InputTruthRingerKeys=[recordable(k) for k in truth_ring_keys],
            InputTruthElectronKey=recordable("TruthElectrons"),
        )
    if enable_muon_system:
        ntuple_kwargs["InputMuonKey"] = recordable("Muons")
    ntuple = RootStreamNtupleMaker("NtupleMaker", **ntuple_kwargs)
    acc += ntuple

    acc.run(events)


if __name__ == "__main__":
    parser = parse_args()
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()
    args = update_args(args)
    pool = create_parallel_job(args)
    pool(main, logging_level=args.output_level, systems=args.systems)
