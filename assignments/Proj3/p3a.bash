#!/bin/bash

awk -f p3a.awk unsortedNames.txt | sort | awk -f p3a.awk - > p3a.out