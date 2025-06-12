#!/bin/bash
set -e

workarea=$1
echo "Iniciando a geração de eventos em $(pwd)"
place=$(pwd)

mkdir -p $workarea 
cd $workarea

# stage 1
if [ ! -f "stage_1.done" ]; then
    /home/administrador/apptainer-1.3.0/lorenzetti/generator/scripts/gen_Upsilon.py --nov 2 --output-level DEBUG -o test.EVT.root
    touch stage_1.done
fi

# stage 2
if [ ! -f "stage_2.done" ]; then
    gen_jets.py --nov 2 -o jets.EVT.root
    touch stage_2.done
fi

# stage 3
if [ ! -f "stage_3.done" ]; then
    gen_minbias.py --nov 1 --pileup-avg 1 -o mb.EVT.root
    touch stage_3.done
fi

# stage 4
if [ ! -f "stage_4.done" ]; then
    simu_trf.py -i test.EVT.root -o jpsi.HIT.root -nt 2
    touch stage_4.done
fi

# stage 5
if [ ! -f "stage_5.done" ]; then
    simu_trf.py -i mb.EVT.root -o mb.HIT.root -nt 2
    touch stage_5.done
fi

# stage 6
if [ ! -f "stage_6.done" ]; then
    merge_trf.py -i jpsi.HIT.root -p mb.HIT.root -o jpsi.merged.HIT.root
    touch stage_6.done
fi

# stage 7
if [ ! -f "stage_7.done" ]; then
    digit_trf.py -i jpsi.HIT.root -o jpsi.ESD.root
    touch stage_7.done
fi

# stage 8
if [ ! -f "stage_8.done" ]; then
    reco_trf.py -i jpsi.ESD.root -o jpsi.AOD.root
    touch stage_8.done
fi

cd $place


