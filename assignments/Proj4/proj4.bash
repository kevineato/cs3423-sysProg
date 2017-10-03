#!/bin/bash

if [ ! -x Emails ]; then
  mkdir Emails
elif [ -d Emails ]; then
  rm -rf Emails
  mkdir Emails
fi

awk -F ',' -v date="$1" -f bashGen.awk p4Customer.txt > gTempBash.bash
bash gTempBash.bash
rm -f gTempBash.bash
