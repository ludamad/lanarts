mkdir -p ../lanarts-eclipse
cd ../lanarts-eclipse

# Ccache speeds up compilation, a LOT
if [ -e /usr/lib/ccache ]; then 
	export PATH=/usr/lib/ccache:$PATH
fi

cmake -G"Eclipse CDT4 - Unix Makefiles" ../lanarts
