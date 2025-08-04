#!/usr/bin/env python3
import argparse
import sys
import os
from pathlib import Path
from typing import List



from expand_folders     import expand_folders
from GaugiKernel        import LoggingLevel, get_argparser_formatter
from GaugiKernel        import ComponentAccumulator
from RootStreamBuilder  import RootStreamHITReader, recordable
from CaloCellBuilder    import PileupMerge
from RootStreamBuilder  import RootStreamHITMaker

from reco.reco_job import merge_args, update_args, create_parallel_job


def parse_args():
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
    parser.add_argument('-o', '--output-file', action='store',
                        dest='output_file', required=True,
                        help="The output file path.")
    parser.add_argument('-i', '--input-file', action='store',
                        dest='input_file', required=True,
                        help="The input HIT file.")
    parser.add_argument('--pileup-file', action='store',
                        dest='pileup_file', required=True,
                        help="The event HIT file for pileup.")
    parser.add_argument('--nov', '--number-of-events', action='store',
                        dest='number_of_events', required=False, default=10,
                        type=int,
                        help="Number of events to process.")
    parser.add_argument('--events-per-job', action='store',
                        dest='events_per_job', required=False,
                        type=int, default=10,
                        help="Number of events per job.")
    parser.add_argument('-m', '--merge', action='store_true',
                        dest='merge', required=False,
                        help="Flag to merge outputs.")
    parser.add_argument('--overwrite', action='store_true',
                        dest='overwrite', required=False,
                        help="Overwrite output files if they exist.")
    parser.add_argument('-nt', '--number-of-threads', action='store',
                        dest='number_of_threads', required=False,
                        type=int, default=1,
                        help="Number of threads to use.")

    return parser


def main(events: List[int],
         input_file: str | Path,
         output_file: str | Path,
         logging_level: str,
         pileup_file: str,
         command: str):

    if isinstance(input_file, Path):
        input_file = str(input_file)
    if isinstance(output_file, Path):
        output_file = str(output_file)

    outputLevel = LoggingLevel.toC(logging_level)

    exec(command)

    acc = ComponentAccumulator("ComponentAccumulator", output_file)

    reader = RootStreamHITReader("HITReader", 
                                InputFile       = input_file,
                                OutputHitsKey   = recordable("Hits"),
                                OutputEventKey  = recordable("Events"),
                                OutputTruthKey  = recordable("Particles"),
                                OutputSeedsKey  = recordable("Seeds"),
                                OutputLevel     = outputLevel,
                                )
    reader.merge(acc)

    pileup = PileupMerge(
       name="PileupMerge",
       InputFile=str(pileup_file),
       InputHitsKey=recordable("Hits"),
       OutputHitsKey="Hits_Merged",
       InputEventKey=recordable("Events"),
       OutputEventKey="Events_Merged",
       OutputLevel=outputLevel
)

    acc += pileup

    HIT = RootStreamHITMaker("RootStreamHITMaker",
                             # input from context
                             InputHitsKey    = "Hits_Merged",
                             InputEventKey   = "Events_Merged",
                             InputTruthKey   = recordable("Particles"),
                             InputSeedsKey   = recordable("Seeds"),
                             # output to file
                             OutputHitsKey   = recordable("Hits"),
                             OutputEventKey  = recordable("Events"),
                             OutputLevel     = outputLevel)
    acc += HIT
    acc.run(events)


def run(args):
    args.pileup_file = Path(args.pileup_file)
    if not args.pileup_file.exists():
        raise FileNotFoundError(f"Pileup input file {args.pileup_file} not found.")
    if args.pileup_file.is_dir():
        files = expand_folders(os.path.abspath(args.pileup_file))
        if len(files) == 0:
            raise FileNotFoundError(f"No pileup files found in directory {args.pileup_file}")
        args.pileup_file = files[0]  # usa só o primeiro arquivo encontrado
    else:
        args.pileup_file = str(args.pileup_file)

    args_dict = vars(args).copy()
    # Remove as chaves que já são passadas explicitamente para evitar conflito
    for key in ["input_file", "output_file", "number_of_events", "nov", "command", "pileup_file"]:
        if key in args_dict:
            del args_dict[key]

    pool = create_parallel_job(args)
    pool(main,
         events=list(range(args.number_of_events)),
         input_file=args.input_file,
         output_file=args.output_file,
         logging_level=args.output_level,
         pileup_file=args.pileup_file,
         command=args.command,
         **args_dict
    )




if __name__ == "__main__":
    parser = parse_args()
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()
    args = update_args(args)
    run(args)

