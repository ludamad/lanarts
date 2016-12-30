if [ -f $1 ] ; then
    moonc $1
    vim ${1/.moon/.lua} $2
    rm ${1/.moon/.lua}
fi
