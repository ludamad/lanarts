if [ $(basename $(pwd)) == 'build-scripts' ] ; then cd .. ; fi

if [ -e ../lanarts_build ]; then
	cd ../lanarts_build
	make clean
fi
