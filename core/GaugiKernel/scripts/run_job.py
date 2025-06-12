import argparse
import multiprocessing
import sys, os, json
from GaugiKernel import get_argparser_formatter

def build_argparser():
    formatter_class = get_argparser_formatter()
    parser = argparse.ArgumentParser(formatter_class=formatter_class)

    subparsers = parser.add_subparsers(dest='option', required=True)

    # Subparser para o comando "run"
    run_parser = subparsers.add_parser("run", help='Executa um job a partir de um JSON.', formatter_class=formatter_class)
    run_parser.add_argument('--job','-j', required=True, type=str, help="Caminho para o arquivo job.json")
    run_parser.add_argument('--output-dir','-o', type=str, default='job_output', help="Diretório onde os arquivos de saída serão gerados")
    run_parser.add_argument('-nt','--number-of-threads', type=int, default=multiprocessing.cpu_count(), help="Número de threads")
    run_parser.add_argument('--force', action='store_true', help="Força a reexecução de etapas já concluídas")

    return parser

def run_job(args):
    # Carrega o job JSON
    job = json.load(open(args.job, 'r'))
    
    # Handle both JSON structures:
    # 1. New style: {"job": {"job_id": 123, ...}, "run": {...}}
    # 2. Old style: {"job_id": 123, "run": {...}, ...}
    job_data = job.get("job", job)  # Use nested "job" if exists, else use root
    run_data = job.get("run", {})   # Get run info from root if not in job
    
    # Get job parameters with defaults
    job_id = job_data.get("job_id", 0)
    seed = job_data.get("seed", 512)
    event_numbers = job_data.get("event_numbers", [])
    run_number = run_data.get("run_number", 20250122)  # Default run number
    
    workarea = os.path.abspath(args.output_dir)
    
    # Substituições de variáveis de ambiente no script
    envs = {
        "%SEED": str(seed),
        "%RUN_NUMBER": str(run_number),
        "%JOB_WORKAREA": workarea,
        "%EVENT_NUMBERS": ",".join([str(evt) for evt in event_numbers]),
        "%CPU_CORES": str(args.number_of_threads),
        "%JOB_ID": str(job_id),
    }

    os.makedirs(workarea, exist_ok=True)
    stages = job.get("stages", [])

    for params in stages:
        step_dir = params.get("name")
        step_path = os.path.join(workarea, step_dir)
        os.makedirs(step_path, exist_ok=True)

        command = params.get("script")
        input_path = params.get("input", None)
        output_path = params.get("output")

        # Construir o comando com os parâmetros
        if input_path:
            command += f" -i {input_path}"
        command += f" -o {output_path}"

        for key, value in params.get("extra_args", {}).items():
             if value == "None":  # Handle string "None" specially
              command += f" --{key}"
             elif value:  # Only add if value exists
               command += f" --{key} {value}"
             else:  # For empty values
               command += f" --{key}"

        # Substituir as variáveis no comando final
        for key, value in envs.items():
            command = command.replace(key, f'"{value}"')

        completed_flag = os.path.join(step_path, 'completed')

        if not os.path.exists(completed_flag) or args.force:
            print(f"\n Executando etapa: {params['name']}")
            print(f" Comando: {command}")
            returncode = os.system(command)
            print(f" Código de retorno: {returncode}")
            if returncode == 0:
                with open(completed_flag, 'w') as f:
                    f.write("completed")
            else:
                print(f" Falha na etapa '{params['name']}' com código {returncode}")
                sys.exit(returncode)
        else:
            print(f" Etapa '{params['name']}' já concluída. Use --force para reexecutar.")

    print("\n Job finalizado com sucesso.")
    sys.exit(0)

def run_parser(args):
    if args.option == "run":
        run_job(args)
    else:
        print(" Opção não implementada")

if __name__ == "__main__":
    parser = build_argparser()
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()
    run_parser(args)
