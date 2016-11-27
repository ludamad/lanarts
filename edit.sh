for i in `find runtime/ src/ -iname '*'"$1"'*'` ; do
    vim $i $2 $3 $4
    break
done
