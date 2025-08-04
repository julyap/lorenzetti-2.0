#!/bin/bash
# -*- coding: utf-8 -*-

# Configure environment
export PYTHONPATH="/home/administrador/apptainer-1.3.0/lorenzetti/core:$PYTHONPATH"
export LC_ALL=C.UTF-8  # Ensure consistent locale



CPU_N=$(grep -c ^processor /proc/cpuinfo)

mkdir EVT && cd EVT

# Generate events 
prun_jobs.py -c "gen_minbias.py -o %OUT --nov %NOV -s %SEED --pileupAvg 2" -nt $CPU_N -o jets.EVT.root --nov 10000 -m

cd ..
