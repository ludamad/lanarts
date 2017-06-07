file=`readlink -f $1`
SCRIPT_DIR=$(readlink -f `dirname $0`)
shift 1
cd `dirname "$SCRIPT_DIR"` ; # Go to parent dir of script dir
file=$(realpath "--relative-to=`pwd`" "$file") # Convert absolute path to script to relative path
./run.sh bare -L "$file" $@ # Run as lua module
