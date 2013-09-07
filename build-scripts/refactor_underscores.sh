function update_file() {
	echo $@
}

for i in $(find . -name '*.lua') ; do
    package='(\@?\.?(?:\w+\.)*)'
    word='([a-z])([a-z]*)'
    IMP='import "'
    replace1=$IMP'$1\u$2$3'
    replace2=$IMP'$1\u$2$3\u$4$5'
    perl -p -i -e "s/$IMP$package$word\"/$replace1\"/g" $i
    perl -p -i -e "s/$IMP$package$word\\_$word\"/$replace2\"/g" $i
  #  echo 'import "test.test_balls"' | perl -pe "s/$IMP$package$word\\_$word\"/$replace\"/g"
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
    mv -f $i "$dname/$fname"
done
