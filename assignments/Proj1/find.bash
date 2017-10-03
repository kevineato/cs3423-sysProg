#!/bin/bash

read -p "Enter customer's apartment number (APT-XX): " aptnum

# Find file containing $aptnum with grep, filename (i.e. email) is assigned to variable
filename=$(grep -rl $aptnum Data)

if [ -z $filename ]; then
  echo -e "\nError: apartment number not found"
  exit 1
fi

. ./read.bash < $filename

echo -e "\nEmail: $email"
echo "Name: $firstname $lastname"
echo "Apt: $apt"
echo "Balance: $balance"
echo "Rent Amt: $rent"
echo "Due Date: $duedate"
