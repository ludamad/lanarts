file=`readlink -f $1`
cd .. ; 
./run.sh bare -L $file 
