#!/usr/bin/env python3
#!/usr/bin/env python3

import argparse
import sys
import os

from typing import List

from evtgen import Pythia8
from filters import Jpsi

from GenKernel import EventTape
from GaugiKernel import get_argparser_formatter
from GaugiKernel import LoggingLevel
from GaugiKernel import GeV

from reco.gen_job import merge_args, update_args, create_parallel_job


# ============================================================
# Configuração
# ============================================================

datapath = os.environ["LORENZETTI_EVTGEN_DATA_DIR"]

JPSI_FILE = f'{datapath}/JPsi_config.cmnd'


"""
Script: gen_jpsi.py

Purpose:
    Generates J/psi -> mu+ mu- events using Pythia8.

Usage:
    gen_jpsi.py -o Jpsi.EVT.root --nov 100
"""


# ============================================================
# Argument parser
# ============================================================

def parse_args():

    parser = argparse.ArgumentParser(
        description='',
        formatter_class=get_argparser_formatter(),
        add_help=False
    )

    parser.add_argument(
        '--run-number',
        action='store',
        dest='run_number',
        required=False,
        type=int,
        default=0,
        help="The run number."
    )

    parser.add_argument(
        '--output-level',
        action='store',
        dest='output_level',
        required=False,
        type=str,
        default="INFO",
        help="The output level messenger."
    )

    parser.add_argument(
        '--eta-max',
        action='store',
        dest='eta_max',
        required=False,
        type=float,
        default=2.7,
        help="The maximum absolute pseudorapidity for muons."
    )

    parser.add_argument(
        '--zero-vertex-particles',
        action='store_true',
        dest='zero_vertex_particles',
        required=False,
        help=(
            "Fix the z vertex position in simulation to zero "
            "for all selected particles."
        )
    )

    parser.add_argument(
        '--jpsi-file',
        action='store',
        dest='jpsi_file',
        required=False,
        type=str,
        default=JPSI_FILE,
        help="The Pythia8 J/psi configuration file."
    )

    return merge_args(parser)


# ============================================================
# Main generation function
# ============================================================

def main(
    events: List[int],
    logging_level: str,
    output_file: str,
    run_number: int,
    seed: int,
    jpsi_file: str,
    zero_vertex_particles: bool,
    eta_max: float
):

    outputLevel = LoggingLevel.toC(logging_level)

    tape = EventTape(
        "EventTape",
        OutputFile=output_file,
        RunNumber=run_number
    )

    jpsi = Jpsi(
        "Jpsi",
        Pythia8(
            "Generator",
            File=jpsi_file,
            Seed=seed
        ),
        EtaMax=eta_max,
        MinPt=3.0,
        ZeroVertexParticles=zero_vertex_particles,
        OutputLevel=outputLevel
    )

    tape += jpsi

    tape.run(events)


# ============================================================
# Execution
# ============================================================

if __name__ == "__main__":

    parser = parse_args()

    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)

    args = parser.parse_args()
    args = update_args(args)

    pool = create_parallel_job(args)

    pool(
        main,
        logging_level=args.output_level,
        run_number=args.run_number,
        jpsi_file=args.jpsi_file,
        zero_vertex_particles=args.zero_vertex_particles,
        eta_max=args.eta_max
    )

