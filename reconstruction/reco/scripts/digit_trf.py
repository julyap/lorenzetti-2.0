#!/usr/bin/env python3
import argparse
import sys
import os

from pathlib            import Path
from typing             import List
from CaloCellBuilder    import CaloCellBuilder
from GaugiKernel        import LoggingLevel, get_argparser_formatter
from GaugiKernel        import ComponentAccumulator
from RootStreamBuilder  import RootStreamHITReader, recordable
from RootStreamBuilder  import RootStreamESDMaker

from reco.reco_job import merge_args, update_args, create_parallel_job
from geometry import DetectorConstruction_v1, DetectorConstruction_v2


try:
    from MuonCellBuilder import MuonCellBuilder
    HAS_MUON_CELL_BUILDER = True
except ImportError:
    MuonCellBuilder = None
    HAS_MUON_CELL_BUILDER = False


"""
Script: digit_trf.py
Purpose: Performs the digitization step in the simulation chain.
         Converts Geant4 energy hits into digital signals (cells) by simulating
         electronic logical pulses, noise, and cross-talk.
Usage:
    digit_trf.py -i input.HIT.root -o output.ESD.root [--systems {calorimeter,muon,atlas}]
"""

def parse_args():
    """
    Parses command-line arguments for the digitization job.

    Returns:
        argparse.Namespace: Configuration arguments including logging level and execution hooks.
    """
    # create the top-level parser
    parser = argparse.ArgumentParser(
        description='',
        formatter_class=get_argparser_formatter(),
        add_help=False)

    parser.add_argument('-l', '--output-level', action='store',
                        dest='output_level', required=False,
                        type=str, default='INFO',
                        help="The output level messenger.")

    parser.add_argument('--pre-init', action='store',
                        dest='pre_init', required=False, default="''",
                        help="The preinit command")

    parser.add_argument('--pre-exec', action='store',
                        dest='pre_exec', required=False, default="''",
                        help="The preexec command")

    parser.add_argument('--post-exec', action='store',
                        dest='post_exec', required=False, default="''",
                        help="The postexec command")

    parser.add_argument('--save-all-cells', action='store_true',
                        dest='save_all_cells', required=False,
                        help="Save all cells in the ESD. By default only the cells inside the RoI around the truth particles are "
                             "saved, and an event without truth particles (minimum bias) gives an empty ESD.")

    parser.add_argument('--detector', action='store',
                        dest='detector', required=False,
                        type=str, default='v1', choices=['v1', 'v2'],
                        help="The detector construction: v1, the original geometry, or v2, the ATLAS geometry; the options of the "
                             "ATLAS geometry need v2.")
    parser.add_argument('--tile-atlas-geometry', action=argparse.BooleanOptionalAction, default=None,
                        dest='tile_atlas_geometry', required=False,
                        help="Use the ATLAS-like tile calorimeter geometry (see simu_trf.py). It must match "
                             "the option used in the simulation, since the tile cells depend on it. On by default with --detector v2 (--no-tile-atlas-geometry switches it off), off with v1.")
    parser.add_argument('--atlas-emb', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_emb', required=False,
                        help="Barrel EM calorimeter with the ATLAS absorber composition (see simu_trf.py). It must "
                             "match the option used in the simulation, since the barrel cells depend on it. On by default with --detector v2 (--no-atlas-emb switches it off), off with v1.")
    parser.add_argument('--atlas-emb-cells', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_emb_cells', required=False,
                        help="Barrel EM with the ATLAS length and cells (see simu_trf.py; needs --atlas-emb). It must "
                             "match the option used in the simulation, since the barrel cells depend on it. On by default with --detector v2 (--no-atlas-emb-cells switches it off), off with v1.")
    parser.add_argument('--atlas-hec', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_hec', required=False,
                        help="HEC as in ATLAS (see simu_trf.py). It must match the option used in the simulation, "
                             "since the HEC samplings and cells depend on it. On by default with --detector v2 (--no-atlas-hec switches it off), off with v1.")
    parser.add_argument('--atlas-emec', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_emec', required=False,
                        help="EM end-cap as in ATLAS (see simu_trf.py). It must match the option used in the simulation, "
                             "since the EMEC cells depend on it. On by default with --detector v2 (--no-atlas-emec switches it off), off with v1.")
    parser.add_argument('--tile-atlas-cells', action=argparse.BooleanOptionalAction, default=None,
                        dest='tile_atlas_cells', required=False,
                        help="Tile cells as in ATLAS (see simu_trf.py). It must match the option used in the simulation, "
                             "since the tile cells depend on it. On by default with --detector v2 (--no-tile-atlas-cells switches it off), off with v1.")

    parser.add_argument('--enable-solenoid-field', action=argparse.BooleanOptionalAction, default=None,
                        dest='enable_solenoid_field', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-enable-solenoid-field switches it off), off with v1.")
    parser.add_argument('--atlas-material-in-front', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_material_in_front', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-material-in-front switches it off), off with v1.")
    parser.add_argument('--tile-atlas-itc', action=argparse.BooleanOptionalAction, default=None,
                        dest='tile_atlas_itc', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-tile-atlas-itc switches it off), off with v1.")
    parser.add_argument('--atlas-fcal', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_fcal', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-fcal switches it off), off with v1.")
    parser.add_argument('--atlas-fcal-cells', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_fcal_cells', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-fcal-cells switches it off), off with v1.")
    parser.add_argument('--atlas-fcal-frozen-showers', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_fcal_frozen_showers', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-fcal-frozen-showers switches it off), off with v1.")
    parser.add_argument('--atlas-g4', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_g4', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-g4 switches it off), off with v1.")
    parser.add_argument('--tile-atlas-gap', action=argparse.BooleanOptionalAction, default=None,
                        dest='tile_atlas_gap', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-tile-atlas-gap switches it off), off with v1.")
    parser.add_argument('--atlas-endcap-cryostat', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_endcap_cryostat', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-endcap-cryostat switches it off), off with v1.")
    parser.add_argument('--atlas-barrel-cryostat', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_barrel_cryostat', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-barrel-cryostat switches it off), off with v1.")
    parser.add_argument('--atlas-run4', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_run4', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-run4 switches it off), off with v1.")
    parser.add_argument('--atlas-forward-shielding', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_forward_shielding', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-forward-shielding switches it off), off with v1.")
    parser.add_argument('--atlas-full-field', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_full_field', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-full-field switches it off), off with v1.")
    parser.add_argument('--atlas-barrel-gap-services', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_barrel_gap_services', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-barrel-gap-services switches it off), off with v1.")
    parser.add_argument('--tile-atlas-run4', action=argparse.BooleanOptionalAction, default=None,
                        dest='tile_atlas_run4', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-tile-atlas-run4 switches it off), off with v1.")
    parser.add_argument('--atlas-emec-argon-fraction', action=argparse.BooleanOptionalAction, default=None,
                        dest='atlas_emec_argon_fraction', required=False,
                        help="As in simu_trf.py; use the same value as in the simulation. On by default with --detector v2 (--no-atlas-emec-argon-fraction switches it off), off with v1.")
    parser.add_argument('--atlas-run4-geometry', action='store', dest='atlas_run4_geometry', required=False, type=str, default="",
                        help="As in simu_trf.py (default: the environment variable LORENZETTI_ATLAS_RUN4_GEOMETRY).")
    parser.add_argument('--atlas-field-map', action='store', dest='atlas_field_map', required=False, type=str, default="",
                        help="As in simu_trf.py (default: the environment variable LORENZETTI_FIELD_MAP).")
    parser.add_argument('--atlas-fcal-electrodes', action='store', dest='atlas_fcal_electrodes', required=False,
                        type=str, default=os.environ.get('LORENZETTI_FCAL_ELECTRODES',
                                  os.path.join(os.environ.get('LORENZETTI_GEOMETRY_DATA_DIR', ''), 'LArFCalElectrodes-06.csv')),
                        help="As in simu_trf.py.")

    # ===== SISTEMAS DE DETECÇÃO =====
    parser.add_argument('--systems', action='store',
                        dest='systems', required=False,
                        type=str, default='calorimeter',
                        choices=['calorimeter', 'muon', 'atlas'],
                        help="Which detector systems are digitized in this job. "
                             "'calorimeter' (default): only CaloCellBuilder (the official behavior). "
                             "'muon': only MuonCellBuilder (calorimeter disabled). "
                             "'atlas': CaloCellBuilder + MuonCellBuilder (both systems). "
                             "This is orthogonal to --detector v1/v2 (geometry). "
                             "The 'muon' and 'atlas' modes require the MuonCellBuilder module "
                             "to be importable (reconstruction/MuonChamber/MuonCellBuilder/python on PYTHONPATH). "
                             "It does NOT filter particles: the input hits are the same as produced by simu_trf.py.")

    parser = merge_args(parser)

    return parser


def main(events : List[int],
         logging_level: str,
         input_file: str | Path,
         output_file: str | Path,
         pre_init: str,
         pre_exec: str,
         post_exec: str,
         detector: str = 'v1',
         atlas_options: dict = None,
         save_all_cells: bool = False,
         systems: str = 'calorimeter',
        ):
    """
    Main function for the digitization process.

    Reads Hits from the input file, simulates the calorimeter and/or muon chamber
    readout electronics (CaloCellBuilder and/or MuonCellBuilder), and produces an
    Event Summary Data (ESD) file containing the corresponding cells.

    Args:
        events (List[int]): List of event indices to process.
        logging_level (str): Logging verbosity.
        input_file (str | Path): Path to input HIT file.
        output_file (str | Path): Path to output ESD file.
        pre_init (str): Hook for pre-initialization code.
        pre_exec (str): Hook for pre-execution code.
        post_exec (str): Hook for post-execution code.
        detector (str): v1, the original geometry, or v2, the ATLAS geometry.
        atlas_options (dict): With v2, the arguments of DetectorConstruction_v2 (the ATLAS options, as in the simulation).
        save_all_cells (bool): If True, saves all cells in the ESD regardless of the RoI.
        systems (str): Which detector systems are digitized: 'calorimeter' (default),
                       'muon' or 'atlas' (both). Orthogonal to the detector geometry.
                       Does NOT filter particles.
    """

    if isinstance(input_file, Path):
        input_file = str(input_file)
    if isinstance(output_file, Path):
        output_file = str(output_file)

    # ===== SISTEMAS DE DETECÇÃO =====
    enable_calorimeter = systems in ('calorimeter', 'atlas')
    enable_muon_system = systems in ('muon', 'atlas')
    if enable_muon_system and not HAS_MUON_CELL_BUILDER:
        raise ImportError(
            "--systems '%s' requested, but MuonCellBuilder could not be imported. "
            "Make sure reconstruction/MuonChamber/MuonCellBuilder/python is on PYTHONPATH." % systems
        )

    outputLevel = LoggingLevel.toC(logging_level)

    exec(pre_init)

    acc = ComponentAccumulator("ComponentAccumulator", output_file)

    # ===== LEITOR DE HITS =====
    # Mantém a chave oficial "Hits" para o calorímetro e adiciona "MuonHits" para múons.
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

    # ===== GEOMETRIA (compartilhada pelos builders de calorímetro e múons) =====
    # A instância é criada uma única vez e reutilizada. O CaloCellBuilder e o
    # MuonCellBuilder devem enxergar a mesma geometria.
    detector_instance = (
        DetectorConstruction_v2("ATLAS", **(atlas_options or {}))
        if detector == 'v2' else DetectorConstruction_v1("ATLAS")
    )

    # ===== CALORÍMETRO =====
    if enable_calorimeter:
        calorimeter = CaloCellBuilder("CaloCellBuilder",
                                      detector_instance,
                                      HistogramPath="Expert/Cells",
                                      OutputLevel=outputLevel,
                                      InputHitsKey=recordable("Hits"),
                                      OutputCellsKey=recordable("Cells"),
                                      OutputTruthCellsKey=recordable("TruthCells"),
                                      InputEventKey=recordable("Events"),
                                      )
        calorimeter.merge(acc)

    # ===== CÂMARA DE MÚONS =====
    muon_cell_keys = []
    if enable_muon_system:
        muon_builder = MuonCellBuilder("MuonCellBuilder",
                                       detector=detector_instance,
                                       HistogramPath="Expert/MuonCells",
                                       InputHitsKey=recordable("MuonHits"),
                                       OutputCellsKey=recordable("MuonCells"),
                                       OutputTruthCellsKey=recordable("TruthMuonCells"),
                                       OutputLevel=outputLevel,
                                       )
        muon_builder.merge(acc)
        if hasattr(muon_builder, 'get_muon_cell_keys'):
            muon_cell_keys = muon_builder.get_muon_cell_keys()

    # ===== ESCRITOR ESD =====
    esd_kwargs = {
        "InputEventKey": recordable("Events"),
        "InputTruthKey": recordable("Particles"),
        "InputSeedsKey": recordable("Seeds"),
        "InputCaloCellsKey": recordable("Cells") if enable_calorimeter else "",
        "InputTruthCaloCellsKey": recordable("TruthCells") if enable_calorimeter else "",
        "InputMuonCellsKey": recordable("MuonCells") if enable_muon_system else "",
        "InputTruthMuonCellsKey": recordable("TruthMuonCells") if enable_muon_system else "",
        "OutputEventKey": "EventInfo",
        "OutputTruthKey": "Particles",
        "OutputSeedsKey": "Seeds",
        "OutputCaloCellsKey": "Cells",
        "OutputTruthCaloCellsKey": "TruthCells",
        "OutputMuonCellsKey": "MuonCells",
        "EnableCaloOutput": enable_calorimeter,
        "EnableMuonOutput": enable_muon_system,
        "OnlyRoI": not save_all_cells,
        "OutputLevel": outputLevel,
    }
    if muon_cell_keys:
        esd_kwargs["InputMuonCellKeys"] = muon_cell_keys
    ESD = RootStreamESDMaker("RootStreamESDMaker", **esd_kwargs)
    acc += ESD

    exec(pre_exec)
    acc.run(events)
    exec(post_exec)


if __name__ == "__main__":
    parser = parse_args()
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()
    # defaults of the ATLAS options (F41, 05/11/2026): on with --detector v2, off with v1, as in simu_trf.py
    atlas_options = ['enable_solenoid_field', 'tile_atlas_geometry', 'tile_atlas_cells', 'atlas_material_in_front', 'tile_atlas_itc', 'atlas_emb', 'atlas_emb_cells', 'atlas_hec', 'atlas_emec', 'atlas_fcal', 'atlas_fcal_cells', 'atlas_fcal_frozen_showers', 'atlas_g4', 'tile_atlas_gap', 'atlas_endcap_cryostat', 'atlas_barrel_cryostat', 'atlas_run4', 'atlas_forward_shielding', 'atlas_full_field', 'atlas_barrel_gap_services', 'tile_atlas_run4', 'atlas_emec_argon_fraction']
    if args.detector == 'v1':
        v2_only = [o for o in atlas_options if getattr(args, o)]
        if v2_only:
            parser.error('--' + v2_only[0].replace('_', '-') + ' is an option of the ATLAS geometry: it needs --detector v2')
    for o in atlas_options:
        if getattr(args, o) is None:
            setattr(args, o, args.detector == 'v2')
    if args.detector == 'v2' and args.atlas_run4:
        for value, option, env in ((args.atlas_run4_geometry, '--atlas-run4-geometry', 'LORENZETTI_ATLAS_RUN4_GEOMETRY'),
                                   (args.atlas_field_map, '--atlas-field-map', 'LORENZETTI_FIELD_MAP')):
            f = value or os.environ.get(env, '')
            if not os.path.isfile(f):
                parser.error("--atlas-run4 (on by default with --detector v2) needs %s or the environment variable %s: '%s' not "
                             "found; switch the Run 4 parts off as in the simulation" % (option, env, f))
    # the arguments of DetectorConstruction_v2
    camel = lambda o: 'UseSolenoidField' if o == 'enable_solenoid_field' else ''.join(w.capitalize() for w in o.split('_'))
    args.atlas_kwargs = { camel(o): getattr(args, o) for o in atlas_options }
    args.atlas_kwargs.update( AtlasRun4Geometry=args.atlas_run4_geometry, AtlasFieldMap=args.atlas_field_map,
                              AtlasFcalElectrodes=args.atlas_fcal_electrodes )

    # ===== SISTEMAS: validação cruzada =====
    if args.systems in ('muon', 'atlas') and not HAS_MUON_CELL_BUILDER:
        parser.error("--systems %s requires the MuonCellBuilder module "
                     "(reconstruction/MuonChamber/MuonCellBuilder/python on PYTHONPATH)" % args.systems)
    if args.systems == 'muon' and args.save_all_cells:
        parser.error("--systems muon does not support --save-all-cells (no calorimeter cells)")

    args = update_args(args)
    pool  = create_parallel_job(args)
    pool( main,
         logging_level    = args.output_level,
         pre_init         = args.pre_init,
         pre_exec         = args.pre_exec,
         post_exec        = args.post_exec,
         detector         = args.detector,
         atlas_options    = args.atlas_kwargs if args.detector == 'v2' else None,
         save_all_cells   = args.save_all_cells,
         systems          = args.systems,
         )
