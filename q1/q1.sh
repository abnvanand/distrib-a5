if [[ $# -lt 2 ]]; then
  echo "Usage: bash q1.sh num_process num_elements elements separated by spaces"
  exit 1
fi
# $1 = n_procs

#echo "$1"
cmdargs=("$@")
n_elements=${cmdargs[1]}
#echo "${cmdargs[@]:1:$n_elements+1}"
mpic++ main.cpp
# cmsargs[1] = array size
# cmdargs[2: end] = array elements
echo "${cmdargs[@]:1:$n_elements+1}" | mpirun --oversubscribe -np "$1" a.out
