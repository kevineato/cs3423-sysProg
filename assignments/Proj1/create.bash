#!/bin/bash

read -p "Enter customer e-mail: " email
read -p "Enter customer's full name: " name
read -p "Enter customer's apartment number (APT-XX): " aptnum
read -p "Enter the monthly rent amount: " rent
read -p "Enter the next due date for rent (YYYY-MM-DD): " duedate

# Check if file already exists
if [ -e "Data/$email" ]; then
  echo -e "\nError: customer already exists"
  exit 1
fi

echo "$email $name" > "Data/$email"
echo "$aptnum $rent 0.00 $duedate" >> "Data/$email"