#!/bin/bash

if [ $# != 1 ]; then
  echo "Usage: $0 DATE"
  exit 1
fi

if [ ! -x Emails ]; then
  mkdir Emails
elif [ -d Emails ]; then
  rm -rf Emails
  mkdir Emails
fi

awk -F ',' -v date="$1" -f bashGen.awk p4Customer.txt > gTempBash.bash
bash gTempBash.bash
rm -f gTempBash.bash
