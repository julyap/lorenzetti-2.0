#!/usr/bin/env python3

import argparse
import sys, os, traceback

from GaugiKernel import get_argparser_formatter
from GaugiKernel import LoggingLevel
from GaugiKernel import GeV
from filters import Upsilon


def build_argparser():
    parser = argparse.ArgumentParser(description='', formatter_class=get_argparser_formatter(), add_help=False)

    parser.add_argument('-e', '--event-numbers', action='store', dest='event_numbers', required=False, type=str, default=None,
                        help="The event number list separated by ','. e.g. --event-numbers '0,1,2,3'")
    parser.add_argument('-o', '--output-file', action='store', dest='output_file', required=True,
                        help="The event file generated by pythia.")
    parser.add_argument('--nov', '--number-of-events', action='store', dest='nov', required=False, type=int, default=1,
                        help="The number of events to be generated.")
    parser.add_argument('--run-number', action='store', dest='run_number', required=False, type=int, default=0,
                        help="The run number.")
    parser.add_argument('-s', '--seed', action='store', dest='seed', required=False, type=int, default=0,
                        help="The pythia seed (zero is the clock system)")
    parser.add_argument('--eta-max', action='store', dest='eta_max', required=False, type=float, default=3.2,
                        help="The eta max used in generator.")
    parser.add_argument('--output-level', action='store', dest='output_level', required=False, type=str, default="INFO",
                        help="The output level messenger.")
    parser.add_argument('--force-forward-electron', action='store_true', dest='force_forward_electron', required=False,
                        help="Force at least one electron into forward region.")
    parser.add_argument('--zero-vertex-particles', action='store_true', dest='zero_vertex_particles', required=False,
                        help="Fix the z vertex position in simulation to zero for all selected particles. It is applied only at G4 step, not in generation.")
    parser.add_argument('--pileup-avg', action='store', dest='pileup_avg', required=False, type=int, default=0,
                        help="The pileup average (default is zero).")
    parser.add_argument('--pileup-sigma', action='store', dest='pileup_sigma', required=False, type=int, default=0,
                        help="The pileup sigma (default is zero).")
    parser.add_argument('--bc-id-start', action='store', dest='bc_id_start', required=False, type=int, default=-21,
                        help="The bunch crossing id start.")
    parser.add_argument('--bc-id-end', action='store', dest='bc_id_end', required=False, type=int, default=4,
                        help="The bunch crossing id end.")
    parser.add_argument('--bc-duration', action='store', dest='bc_duration', required=False, type=int, default=25,
                        help="The bunch crossing duration (in nanoseconds).")
    return parser


def run(args):
    outputLevel = LoggingLevel.toC(args.output_level)

    try:
        from evtgen import Pythia8
        from GenKernel import EventTape

        tape = EventTape("EventTape", OutputFile=args.output_file, RunNumber=args.run_number)

        main_file = os.environ['LZT_PATH'] + '/generator/evtgen/data/Upsilon_config.cmnd'


        upsilon = Upsilon("Upsilon", 
                  Pythia8("Generator", 
                          File=main_file, 
                          Seed=args.seed),
                  EtaMax               = args.eta_max,
                  MinPt                = 15*GeV,
                  ZeroVertexParticles  = args.zero_vertex_particles, #calibration use only.
                  
                  OutputLevel          = outputLevel
                 )
        tape += upsilon


        if args.pileup_avg > 0:
            mb_file = os.environ['LZT_PATH'] + '/generator/evtgen/data/minbias_config.cmnd'

            from filters import Pileup
            pileup = Pileup("Pileup",
                            Pythia8("MBGenerator", File=mb_file, Seed=args.seed),
                            EtaMax=args.eta_max,
                            Select=2,
                            PileupAvg=args.pileup_avg,
                            PileupSigma=args.pileup_sigma,
                            BunchIdStart=args.bc_id_start,
                            BunchIdEnd=args.bc_id_end,
                            OutputLevel=outputLevel,
                            DeltaEta=0.22,
                            DeltaPhi=0.22,
                           )

            tape += pileup

        # Run!
        evts = [int(evt) for evt in args.event_numbers.split(",")] if args.event_numbers else args.nov
        tape.run(evts)

        sys.exit(0)
    except Exception:
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    parser = build_argparser()
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()
    run(args)

