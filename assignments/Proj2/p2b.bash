#!/bin/bash

log1file='log1.tmp'
log2file='log2.tmp'

sed -E -f p2a.sed lastlog1.out > $log1file
sed -E -f p2a.sed lastlog2.out > $log2file

cat $log1file $log2file | sort | uniq -c | sed -E -f p2b.sed > p2b.out

rm -f $log1file $log2file
