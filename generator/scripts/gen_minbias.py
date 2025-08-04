#!/usr/bin/env python3
from GaugiKernel import Logger, get_argparser_formatter
import argparse
import sys, os, traceback

mainLogger = Logger.getModuleLogger("pythia")

def main():
    parser = argparse.ArgumentParser(description='', formatter_class=get_argparser_formatter())
    
    # Argumentos
    parser.add_argument('-e','--event-numbers', dest='event_numbers', default=None,
                       help="Lista de números de eventos separados por vírgula")
    parser.add_argument('-o','--output-file', dest='output_file', required=True,
                       help="Arquivo de saída")
    parser.add_argument('--nov','--number-of-events', dest='nov', type=int, default=1,
                       help="Número de eventos")
    parser.add_argument('--run-number', dest='run_number', type=int, default=0,
                       help="Número da execução")
    parser.add_argument('-s','--seed', dest='seed', type=int, default=0,
                       help="Semente para Pythia")
    parser.add_argument('--output-level', dest='output_level', default="INFO",
                       help="Nível de log")
    parser.add_argument('--pileup-avg', dest='pileup_avg', type=int, default=0,
                       help="Média de pileup")
    parser.add_argument('--pileup-sigma', dest='pileup_sigma', type=int, default=0,
                       help="Sigma do pileup")
    parser.add_argument('--bc-id-start', dest='bc_id_start', type=int, default=-21,
                       help="ID inicial de bunch crossing")
    parser.add_argument('--bc-id-end', dest='bc_id_end', type=int, default=4,
                       help="ID final de bunch crossing")
    parser.add_argument('--bc-duration', dest='bc_duration', type=int, default=25,
                       help="Duração do bunch crossing (ns)")

    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)

    args = parser.parse_args()

    try:
        # Configuração do Pythia
        minbias_file = os.path.join(os.environ['LZT_PATH'], 'generator/evtgen/data/minbias_config.cmnd')
        
        from evtgen import Pythia8
        from GenKernel import EventTape
        from filters import FixedRegion, Pileup

        # Configuração da fita de eventos
        tape = EventTape("EventTape", OutputFile=args.output_file, RunNumber=args.run_number)
        
        # Semente fixa para partículas de pileup
        seed = FixedRegion("Seed", Eta=0, Phi=0)
        tape += seed
        
        # Gerador de pileup
        pileup = Pileup(
            "MinimumBias",
            Pythia8("Generator", File=minbias_file, Seed=args.seed),
            EtaMax=1.4,
            Select=2,
            PileupAvg=args.pileup_avg,
            PileupSigma=args.pileup_sigma,
            BunchIdStart=args.bc_id_start,
            BunchIdEnd=args.bc_id_end,
            OutputLevel=args.output_level,
            DeltaEta=999,
            DeltaPhi=999,
        )
        tape += pileup

        # Execução
        evts = [int(evt) for evt in args.event_numbers.split(",")] if args.event_numbers else args.nov
        tape.run(evts)
        
        # Relatório de erros (modificado para evitar AttributeError)
        print("\nResumo de erros:")
        if hasattr(tape, 'counters'):
            errors = tape.counters.get('HadronLevelError', 0)
            mainLogger.info(f"Eventos com falha na hadronização: {errors}")
        else:
            mainLogger.warning("Sistema de contadores não disponível")

        sys.exit(0)
        
    except Exception as e:
        mainLogger.error(f"Erro na execução: {str(e)}")
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()
