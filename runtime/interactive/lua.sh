file=`readlink -f $1`
cd .. ; 
file=$(realpath "--relative-to=`pwd`" "$file")
./run.sh bare -L "$file"
