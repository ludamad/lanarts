function update_file() {
	echo $@
}

for i in $(find . -name '*.lua') ; do
    package='(@?\.?(?:\w+\.)*)'
    word='([a-z])([a-z]*)'
    IMP='import "'
    replace=$IMP'$1\u$2$3\u$4$5'
    perl -p -i -e "s/$IMP$package$word_$word\"/$replace\"/g" $i
#    echo $res > $i
done

for i in $(find . -name '*.lua') ; do
    dname=$(dirname $i)
    fname=$(basename $i)
    word='([A-Za-z])([a-z]*)'
    replace='\u$1$2'
    fname=$(echo $fname | perl -pe "s/$word/$replace/g")
    fname=$(echo $fname | sed 's/_//g')
    fname=$(echo $fname | sed 's/Lua/lua/g')
    echo "New filename $fname"
    mv $i "$dname/$fname"
#    echo $res > $i
done

# perl -pe 's/import "(\.?(?:\w+\.)*)([a-z])([a-z]*)_([a-z])([a-z]*)"/ and"import \"$1\u$2$3\u$4$5\"\int'
