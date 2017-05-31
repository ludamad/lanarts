file=`readlink -f $1`
cd .. ; 
LANARTS_HEADLESS=1 ./run.sh -O bare -L $file
