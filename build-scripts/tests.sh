if [ $(basename $(pwd)) == 'build-scripts' ] ; then cd .. ; fi

if ! ./build-scripts/make.sh; then
        echo "Build failed, aborting lanarts run."
        exit $?
fi

function sgrep {
        local sedarg reset word;
        sedarg=""
        for wordcol in "$@"; do
                word=$(echo ${wordcol%|*} | sed 's/ *$//')
                if [ "$word" == "" ] ; then continue; fi
                if [ "${arg:0:1}" == '.' ] ; then continue; fi
                col=$(echo -e "\e[${wordcol#*|}m")
                reset=$(echo -e "\e[0m")
                sedarg+="s/\($word\)/$col\1$reset/gi;"
        done
        sed "$sedarg"
}

function highlight {
        sgrep  'failed|1;31' 'failure|1;31' 'warning.*|1;31' 'error.*|1;31' ' passed|1;36'
}

# Run lanarts tests

cd ../lanarts
../lanarts_build/src/lanarts/lanarts_tests | highlight

