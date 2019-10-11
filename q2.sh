if [[ $# -lt 2 ]]; then
  echo "bash q2.sh num_processes(2n+1)
n(# of men/women)
n lines containing prefs of men
n lines containing prefs of women
"
  exit 1
fi
# $1 = n_procs

#echo "n_procs"
#echo "$1"

cmdargs=("$@")
n=${cmdargs[1]}
n_elements=$((n * n * 2))
#echo "n_elements"
#echo "$n_elements"

#echo "${cmdargs[@]:1:10}"
mpic++ main.cpp
# cmsargs[1] = array size
# cmdargs[2: end] = array elements
#echo "${cmdargs[@]:1:$n_elements+1}"
echo "${cmdargs[@]:1:$n_elements+1}" | mpirun --oversubscribe -np "$1" a.out
