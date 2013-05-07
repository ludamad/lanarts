if [ $(basename $(pwd)) == 'build-scripts' ] ; then cd .. ; fi

mkdir -p ../lanarts-eclipse
cd ../lanarts-eclipse

cmake -G"Eclipse CDT4 - Unix Makefiles" ../lanarts
