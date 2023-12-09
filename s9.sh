#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Insufficient arguments"
    exit 1
fi

char=$1
lines_count=0 

while read -r line || [[ -n "$line" ]]; do
    if [[ "$line" =~ ^[[:upper:]][[:alnum:],[:space:].!]*[.,?!]$ && "$line" != *,\ si\ * ]]; //conditiile pentru linia citita
    then
	lines_count=$((lines_count+1))
    fi
done

echo "The number of lines that contain the character '$1': $lines_count"
    
