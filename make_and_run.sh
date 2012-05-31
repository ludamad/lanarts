mkdir ../lanarts_build
cd ../lanarts_build
cmake ../lanarts
make -j4
cd ../lanarts
../lanarts_build/src/lanarts
