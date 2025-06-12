#!/usr/bin/env python3

import argparse
import numpy as np
import multiprocessing
import sys, os, json
from GaugiKernel import get_argparser_formatter, chunks

def build_argparser_create_jobs():
    parser = argparse.ArgumentParser(description='', formatter_class=get_argparser_formatter(), add_help=False)
    parser.add_argument('--production-card', '-p', dest='production_card', required=False, type=str, default=None, help="The production card")
    parser.add_argument('--output', '-o', dest='output', required=False, type=str, default='jobs', help="Output folder for jobs")
    return parser

def build_argparser_run_job():
    parser = argparse.ArgumentParser(description='', formatter_class=get_argparser_formatter(), add_help=False)
    parser.add_argument('--job', '-j', dest='job', required=True, type=str, help="Path to the job JSON file")
    parser.add_argument('--output-dir', '-o', dest='output_dir', required=False, type=str, default='job_output', help="Job output path")
    parser.add_argument('--run-all', action='store_true', help="Run all jobs in the same directory as the job file")
    parser.add_argument('-nt', '--number-of-threads', dest='number_of_threads', required=False, type=int, default=multiprocessing.cpu_count(), help="Number of threads")
    return parser

def run_create_jobs(args):
    prod = json.load(open(args.production_card, 'r'))
    chunk_size = prod["run"]["nov_per_job"]
    seed = prod["run"]["seed"]
    os.makedirs(args.output, exist_ok=True)
    for idx, evts in enumerate(chunks(np.arange(0, prod["run"]["nov"]).tolist(), chunk_size)):
        with open(f"{args.output}/job.{idx}.json", 'w') as f:
            d = prod | {"job": {"event_numbers": evts, "seed": seed * (idx + 1), "job_id": idx}}
            json.dump(d, f, indent=4)

def run_single_job(job_path, number_of_threads, output_dir):
    job = json.load(open(job_path, 'r'))
    job_id = job["job"].get("job_id")
    run_number = job["run"].get("run_number")
    seed = job["job"]["seed"]
    workarea = os.path.abspath(output_dir)

    envs = {
        "%SEED": str(seed),
        "%RUN_NUMBER": str(run_number),
        "%JOB_WORKAREA": workarea,
        "%EVENT_NUMBERS": ",".join([str(evt) for evt in job["job"]["event_numbers"]]),
        "%CPU_CORES": str(number_of_threads),
        "%JOB_ID": str(job_id),
    }

    os.makedirs(workarea, exist_ok=True)
    stages = job.get("stages", [])
    for params in stages:
        step_dir = params.get("name")
        step_path = f"{workarea}/{step_dir}"
        os.makedirs(step_path, exist_ok=True)
        command = params.get("script")
        input_path = params.get("input", None)
        output_path = params.get("output")

        if input_path:
            command += f" -i {input_path}"
        command += f" -o {output_path}"

        for key, value in params.get("extra_args", {}).items():
            command += f" --{key} {value}" if value else f" --{key}"

        for key, value in envs.items():
            command = command.replace(key, value)

        print(f"\n[JOB {job_id}] Running command: {command}")
        if not os.path.exists(f"{step_path}/completed"):
            returncode = os.system(command)
            if returncode == 0:
                with open(f"{step_path}/completed", 'w') as f:
                    f.write("completed")
            else:
                print(f"[JOB {job_id}] Erro ao executar comando.")
                sys.exit(returncode)

def run_job(args):
    # Caminho absoluto para o diretório onde os arquivos de job estão localizados
    job_dir = "/home/administrador/apptainer-1.3.0/lorenzetti/jobs"
    
    # Se o parâmetro --run-all for especificado, executa todos os jobs na pasta 'jobs'
    if args.run_all:
        job_files = sorted([f for f in os.listdir(job_dir) if f.startswith("job.") and f.endswith(".json")])
        for job_file in job_files:
            job_path = os.path.join(job_dir, job_file)
            print(f"\n===> Executando: {job_path}")
            try:
                run_single_job(job_path, args.number_of_threads, args.output_dir)
            except Exception as e:
                print(f"Erro ao executar {job_file}: {e}")
                continue
    else:
        # Caso contrário, executa apenas o job especificado
        run_single_job(args.job, args.number_of_threads, args.output_dir)

def build_argparser():
    parser = argparse.ArgumentParser(formatter_class=get_argparser_formatter())
    subparsers = parser.add_subparsers(dest='option')
    subparsers.add_parser("create", parents=[build_argparser_create_jobs()], help="Create jobs from production card")
    subparsers.add_parser("run", parents=[build_argparser_run_job()], help="Run a job or all jobs")
    return parser

def run_parser(args):
    if args.option == "create":
        run_create_jobs(args)
    elif args.option == "run":
        run_job(args)
    else:
        print("Opção não implementada.")

if __name__ == "__main__":
    parser = build_argparser()
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()
    run_parser(args)

