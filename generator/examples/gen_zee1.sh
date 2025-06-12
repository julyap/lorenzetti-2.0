#!/bin/bash
# -*- coding: utf-8 -*-

# Configure environment
export PYTHONPATH="/home/administrador/apptainer-1.3.0/lorenzetti/core:$PYTHONPATH"
export LC_ALL=C.UTF-8  # Ensure consistent locale

# Set fixed CPU count
CPU_N=54

# Path configurations
JOB_DIR="/home/administrador/apptainer-1.3.0/lorenzetti/EVT/jobs/jobs_zee"
OUTPUT_BASE="/home/administrador/apptainer-1.3.0/lorenzetti/EVT/output"
SCRIPT_PATH="/home/administrador/apptainer-1.3.0/lorenzetti/core/GaugiKernel/scripts/run_job.py"

# Validate directories
mkdir -p "$OUTPUT_BASE" || { echo "Failed to create output directory"; exit 1; }
cd "$JOB_DIR" || { echo "Job directory not found"; exit 1; }

# Job processing function
run_single_job() {
    local job_file="$1"
    local job_id=$(basename "$job_file" | grep -oP 'job\.\K\d+(?=\.json)')
    local output_dir="$OUTPUT_BASE/job_$job_id"
    
    mkdir -p "$output_dir" || return 1
    
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] Starting job $job_id"
    
    if ! python3 "$SCRIPT_PATH" run \
        -j "$job_file" \
        -nt 1 \
        -o "$output_dir" > "$output_dir/job_${job_id}.log" 2>&1; then
        
        echo "[$(date '+%Y-%m-%d %H:%M:%S')] ERROR in job $job_id" >&2
        return 1
    else
        echo "[$(date '+%Y-%m-%d %H:%M:%S')] Completed job $job_id"
        return 0
    fi
}

export -f run_single_job
export OUTPUT_BASE SCRIPT_PATH

# Main execution with xargs
echo "Starting parallel execution with $CPU_N workers"
find "$JOB_DIR" -name 'job.*.json' -print0 | \
    xargs -0 -P "$CPU_N" -I {} bash -c 'run_single_job "$@"' _ {}

# Count results
success_count=0
error_count=0

for log in "$OUTPUT_BASE"/job_*/job_*.log; do
    if [ -f "$log" ]; then
        if grep -q "Completed job" "$log"; then
            ((success_count++))
        elif grep -q "ERROR" "$log"; then
            ((error_count++))
        fi
    fi
done

echo "======================================"
echo "Job processing completed"
echo "Success: $success_count"
echo "Errors: $error_count"
echo "Total: $((success_count + error_count))"
echo "======================================"

exit $((error_count > 0))

