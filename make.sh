mkdir -p ../lanarts_build
cd ../lanarts_build

CC=~/clangllvm/clang+llvm-3.1-x86_64-linux-ubuntu_12.04/bin/clang CXX=~/clangllvm/clang+llvm-3.1-x86_64-linux-ubuntu_12.04/bin/clang++ cmake ../lanarts

## START TIMER CODE
T="$(date +%s%N)"
## START TIMER CODE

##MAKE
make -j4

## END TIMER CODE
T="$(($(date +%s%N)-T))"
echo "Total build time: $((T/1000000))ms"
## END TIMER CODE
