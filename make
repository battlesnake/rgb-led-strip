#!/bin/bash

pushd "`dirname "$0"`" > /dev/null
trap "popd > /dev/null" EXIT

BIN=$PWD/bin
INCLUDE=$PWD/include
OUT=$BIN
OBJ=$PWD/obj
FLAGS="-mcpu=arm1176jzf-s -Wall -O3 -pipe -iquote $INCLUDE"
RUN=

[ -z "$1" ] && {
	echo "Syntax: make <project name>"
	exit 0
}

if [ "$1" == "include" ]
then
	OUT=$PWD/obj
	rm $OUT/*.o &>/dev/null
	FLAGS="$FLAGS -c"
else
	rm $BIN/$1.o &>/dev/null
	FLAGS="`ls $OBJ/*.o` $FLAGS -o $BIN/$1.o"
	RUN=yes
fi

(
	set -vx
	g++ $1/*.c $FLAGS
)
RES=$?

if ! [ $RES == 0 ]
then
	if [ "$RUN" ]
	then
		rm $OUT/*.o &>/dev/null
	else
		rm $OBJ/*.o ./*.o &>/dev/null
	fi
	echo "Compile failed ($RES)"
	exit 0
fi

if [ "$RUN" ]
then
	chmod +x $BIN/$1.o
	$BIN/$1.o
	clear
else
	mv *.o obj/
	echo "Compile successful"
fi
