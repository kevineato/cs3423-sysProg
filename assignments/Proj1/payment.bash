#!/bin/bash

read -p "Enter customer email: " email
read -p "Enter payment amount: " payment

if [ ! -e "Data/$email" ]; then
  echo -e "\nError: customer not found"
  exit 1
fi

. ./read.bash < "Data/$email"

# Calculate balance after subtracting payment
newbalance=$(echo "scale=2; $balance + $payment" | bc)

echo -e "$email $firstname $lastname\n$apt $rent $newbalance $duedate" > "Data/$email"
