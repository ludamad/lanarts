for i in `find $3 -name '*.cpp' -o -name '*.h' -o -name '*.yaml' -o -name '*.lua' -o -name '*.moon'` ; do
    if grep -m 1 "$1" $i > /dev/null ; then
        echo "Changing $i"
        sed -i "s|$1|$2|g" $i
    fi
done
