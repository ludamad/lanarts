
mkdir -p ../lanarts_build
cd ../lanarts_build

cmake ../lanarts

## START TIMER CODE
T="$(date +%s%N)"
## START TIMER CODE

##MAKE
failed=false
if ! make -j4 then
	failed=true
fi

## END TIMER CODE
T="$(($(date +%s%N)-T))"
echo "Total build time: $((T/1000000))ms"
## END TIMER CODE

exit $failed
