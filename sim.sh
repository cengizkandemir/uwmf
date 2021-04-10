#!/bin/bash

imgs_folder="./images"
if [ "$#" -gt 0 ]; then
    imgs_folder="$1"
fi

r=10
if [ "$#" -gt 1 ]; then
    r=$2
fi

output="./results.txt"
if [ "$#" -gt 2 ]; then
    output="$3"
fi

echo "" > $output

corr_dens=(0.1 0.3 0.5 0.7 0.9)
wsizes=(1 2 3 4 6)

for file_name in $imgs_folder/*.png; do
    echo "processing $file_name" | tee -a $output
    for i in ${!corr_dens[@]}; do
        ./uwmf -i $file_name -m s -w ${wsizes[i]} -d ${corr_dens[i]} -r $r 2>>$output
        echo "--------------------" >> $output
    done
    echo "" >> $output
done
