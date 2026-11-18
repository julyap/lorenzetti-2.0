#!/usr/bin/env python3
import argparse
import sys
import os

from pathlib            import Path
from typing             import List
from expand_folders     import expand_folders
from GaugiKernel        import LoggingLevel, get_argparser_formatter
from GaugiKernel        import ComponentAccumulator
from RootStreamBuilder  import RootStreamHITReader, recordable
from CaloCellBuilder    import PileupMerge
from RootStreamBuilder  import RootStreamHITMaker

from reco.reco_job import merge_args, update_args, create_parallel_job



def parse_args():
    # create the top-level parser
    parser = argparse.ArgumentParser(
        description='',
        formatter_class=get_argparser_formatter(),
        add_help=False)

    parser.add_argument('-l', '--output-level', action='store',
                        dest='output_level', required=False,
                        type=str, default='INFO',
                        help="The output level messenger.")
    parser.add_argument('-c', '--command', action='store',
                        dest='command', required=False, default="''",
                        help="The preexec command")
    parser.add_argument('--low-pileup-files', action='store',
                        dest='low_pileup_files', required = True,
                        help = "The event HIT file to be merged (pileup).")
    parser.add_argument('--high-pileup-files', action='store',
                        dest='high_pileup_files', required = True,
                        help = "The event HIT file to be merged (pileup).")
    parser.add_argument('--pileup-avg', action='store',
                        dest='pileup_avg', required=True,
                        type=int,
                        help="The pileup average.")
    parser.add_argument('--pileup-sigma', action='store',
                        dest='pileup_sigma', required=True,
                        type=int,
                        help="The pileup sigma.")

    # ===== SISTEMAS DE DETECÇÃO =====
    parser.add_argument('--systems', action='store',
                        dest='systems', required=False,
                        type=str, default='calorimeter',
                        choices=['calorimeter', 'muon', 'atlas'],
                        help="Which detector systems are present in the input HIT file "
                             "and should be propagated to the merged output. "
                             "'calorimeter' (default): only calorimeter hits (the official behavior); "
                             "pileup is applied to 'Hits'. "
                             "'muon': only muon hits; pileup is NOT applied (PileupMerge is a "
                             "calorimeter-only tool) and 'MuonHits' pass through unchanged. "
                             "'atlas': both; 'Hits' are merged via PileupMerge, and 'MuonHits' "
                             "pass through unchanged (no pileup in the muon spectrometer). "
                             "This flag is orthogonal to the pileup configuration.")

    return merge_args(parser)


def main(events: List[int],
         input_file: str | Path,
         output_file: str | Path,
         logging_level: str,
         low_pileup_files: List[str],
         high_pileup_files: List[str],
         pileup_avg : int,
         pileup_sigma : int,
         command: str,
         systems: str = 'calorimeter'):

    if isinstance(input_file, Path):
        input_file = str(input_file)
    if isinstance(output_file, Path):
        output_file = str(output_file)

    # ===== SISTEMAS =====
    enable_calo = systems in ('calorimeter', 'atlas')
    enable_muon_system = systems in ('muon', 'atlas')

    outputLevel = LoggingLevel.toC(logging_level)

    exec(command)

    acc = ComponentAccumulator("ComponentAccumulator", output_file)

    # ===== LEITOR PRINCIPAL =====
    reader_kwargs = dict(
        InputFile=input_file,
        OutputHitsKey=recordable("Hits"),
        OutputEventKey=recordable("Events"),
        OutputTruthKey=recordable("Particles"),
        OutputSeedsKey=recordable("Seeds"),
        OutputLevel=outputLevel,
    )
    if enable_muon_system:
        reader_kwargs["OutputMuonHitsKey"] = recordable("MuonHits")
        reader_kwargs["EnableMuonInput"] = True
    reader = RootStreamHITReader("HITReader", **reader_kwargs)
    reader.merge(acc)


    if enable_calo:
        pileup = PileupMerge( "PileupMerge",
                              LowPileupInputFiles = low_pileup_files,
                              HighPileupInputFiles= high_pileup_files,
                              PileupAvg           = pileup_avg,
                              PileupSigma         = pileup_sigma,
                              InputHitsKey        = recordable("Hits"),
                              InputEventKey       = recordable("Events"),
                              OutputHitsKey       = "Hits_Merged",
                              OutputEventKey      = "Events_Merged",
                              OutputLevel         = outputLevel
                            )
        acc += pileup

    # ===== ESCRITOR DE HITS =====
    hit_kwargs = dict(
        InputEventKey=recordable("Events"),
        InputTruthKey=recordable("Particles"),
        InputSeedsKey=recordable("Seeds"),
        OutputEventKey=recordable("Events"),
        OutputLevel=outputLevel,
    )
    if enable_calo:
        # Hits mesclados com pileup; Events correspondente
        hit_kwargs["InputHitsKey"]  = "Hits_Merged"
        hit_kwargs["InputEventKey"] = "Events_Merged"
        hit_kwargs["OutputHitsKey"] = recordable("Hits")
    if enable_muon_system:
        # MuonHits passam direto do leitor principal (sem pileup)
        hit_kwargs["InputMuonHitsKey"] = recordable("MuonHits")
        hit_kwargs["EnableMuonOutput"] = True

    HIT = RootStreamHITMaker("RootStreamHITMaker", **hit_kwargs)
    acc += HIT
    acc.run(events)


def run(args):

    args.low_pileup_files  = Path(args.low_pileup_files)
    args.high_pileup_files = Path(args.high_pileup_files)

    if not args.low_pileup_files.exists():
        raise FileNotFoundError(f"Low Pileup input files {args.low_pileup_files} not found.")
    if args.low_pileup_files.is_dir():
        args.low_pileup_files = expand_folders(os.path.abspath(args.low_pileup_files))
    else:
        args.low_pileup_files = [os.path.abspath(args.low_pileup_files)]

    if not args.high_pileup_files.exists():
        raise FileNotFoundError(f"High Pileup input files {args.high_pileup_files} not found.")
    if args.high_pileup_files.is_dir():
        args.high_pileup_files = expand_folders(os.path.abspath(args.high_pileup_files))
    else:
        args.high_pileup_files = [os.path.abspath(args.high_pileup_files)]

    pool = create_parallel_job(args)
    pool( main,
         logging_level    = args.output_level,
         low_pileup_files = args.low_pileup_files,
         high_pileup_files= args.high_pileup_files,
         pileup_avg       = args.pileup_avg,
         pileup_sigma     = args.pileup_sigma,
         command          = args.command,
         systems          = args.systems,
         )



if __name__ == "__main__":
    parser=parse_args()
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()

    # ===== SISTEMAS: validação =====
    if args.systems == 'muon':
        # PileupMerge é uma ferramenta de calorímetro; no modo muon ele é ignorado.
        print("⚠️  --systems muon: PileupMerge will be skipped (for now it is a calorimeter-only tool); "
              "'MuonHits' will pass through unchanged.")

    args = update_args(args)
    run(args)
