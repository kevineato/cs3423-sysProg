#!/bin/bash

echo 'Enter one of the following actions or press CTRL-D to exit.'
echo 'C - create a customer file'
echo 'P - accept a customer payment'
echo 'F - find customer by apartment number'

while read -p 'Enter your choice: ' choice; do
  echo ""
  case "$choice" in
    [Cc]*) ./create.bash;;
    [Pp]*) ./payment.bash;;
    [Ff]*) ./find.bash;;
    *) echo 'Error: invalid action value';;
  esac

  echo -e "\nEnter one of the following actions or press CTRL-D to exit."
  echo 'C - create a customer file'
  echo 'P - accept a customer payment'
  echo 'F - find customer by apartment number'
done
