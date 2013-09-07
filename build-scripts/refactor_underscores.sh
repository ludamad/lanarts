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

# perl -pe 's/import "(\.?(?:\w+\.)*)([a-z])([a-z]*)_([a-z])([a-z]*)"/ and"import \"$1\u$2$3\u$4$5\"\int'
