#!/bin/bash

dollarFile='log1dollar.tmp'
normalFile='log2.tmp'

sed -E -f p2aDollar.sed lastlog1.out > $dollarFile
sed -E -f p2a.sed lastlog2.out > $normalFile

grep -f $dollarFile $normalFile | sort -o p2a.out

rm -f $dollarFile $normalFile
