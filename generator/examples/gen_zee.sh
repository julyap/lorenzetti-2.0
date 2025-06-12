#!/bin/bash
export PYTHONPATH=$PYTHONPATH:/home/administrador/apptainer-1.3.0/lorenzetti/core

# Contar o número de processadores disponíveis
CPU_N=$(grep -c ^processor /proc/cpuinfo)

# Diretórios
JOB_DIR="/home/administrador/apptainer-1.3.0/lorenzetti/EVT/jobs/jobs_zee"
OUTPUT_BASE="/home/administrador/apptainer-1.3.0/lorenzetti/EVT/output"  # Novo: saída organizada
mkdir -p "$OUTPUT_BASE"

# Loop para rodar todos os jobs
cd "$JOB_DIR" || exit 1

for job_file in job.*.json; do
    job_id=$(echo "$job_file" | sed 's/job\.\([0-9]*\)\.json/\1/')
    echo "Executando o job $job_id com o arquivo $job_file..."

    # Cria diretório de saída separado por job
    output_dir="$OUTPUT_BASE/job_$job_id"
    mkdir -p "$output_dir"

    # Executa o script Python de job
    python3 /home/administrador/apptainer-1.3.0/lorenzetti/core/GaugiKernel/scripts/run_job.py run \
        -j "$job_file" \
        -nt "$CPU_N" \
        -o "$output_dir"

    # Verifica sucesso
    if [ $? -ne 0 ]; then
        echo " Erro ao executar o job $job_id"
        break
    else
        echo " Job $job_id finalizado com sucesso"
    fi
done

