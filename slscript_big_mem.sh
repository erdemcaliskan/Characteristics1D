#!/bin/bash
#This file is a submission script to request the ISAAC resources from Slurm
#SBATCH -J Micro_out			       #The name of the job
#SBATCH -A ACF-UTK0011              # The project account to be charged
#SBATCH --nodes=1                     # Number of nodes
#SBATCH --ntasks-per-node=56        # cpus per node
#SBATCH --partition=long-bigmem          # If not specified then default is "campus"
#SBATCH --time=6-00:00:00             # Wall time (days-hh:mm:ss)
#SBATCH --error=job.e%J	       # The file where run time errors will be dumped
#SBATCH --output=job.o%J	       # The file where the output of the terminal will be dumped
#SBATCH --qos=long-bigmem
#SBATCH --nodelist=ilm0835

####------ ACF mpich ------:

for ((i=0; i<=55; i++))
do
	sh script${i}.sh > output_${i}.txt &
done
wait
############ end of PBSscript ##########
