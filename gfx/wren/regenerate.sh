#!/bin/sh

BASEDIR=$(dirname "$0")

cd $BASEDIR # xxd file names are relative to working directory

rm -rf c/*

for f in *.wren
do
   filename=$( basename "$f" .wren)
    # null-terminate strings, remove unsigned
   xxd -i "$f" | sed 's/\([0-9a-f]\)$/\0, 0x00/' | sed 's/unsigned //' > "c/$filename.inc.h"
done
