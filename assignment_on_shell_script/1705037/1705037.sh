#!/bin/bash
# If the user does not provide any working directory name, consider your script is lying in the root working directory
directory=$1
if [ ! -d $1 ]; then
    echo "no working directory provided. Taking root as directory"
    directory="."
fi

# If the user does not provide any input file name, show him a usage message (i.e. how to use this script of yours)
if [ -z "$2" ]; then
    echo "Usage message :"
    echo "Write the following command on command line :"
    echo "./<your_script_name>.sh <your working directory> <input_file_name>"
    exit 0
fi

# Read the types of files to ignore from the input file (if the input file does not exist, prompt the user to give a valid input file name)
if [ ! -f "$2" ]; then
    echo "please give a valid input file name."
    exit 0
fi

n=1
skip_files=""
while read line || [ -n "$line" ]
do
    # reading each line
    # echo "Line No. $n : $line"
    line=${line//[$' \t\r\n']}
    if [ $n -lt 2 ] ; then
        new=" -name *.${line}"
    else
        new=" -o -name *.${line}"
    fi
    skip_files=${skip_files}${new}
    n=$((n+1))
done < $2


# Gather all the required files in a separate output directory (created by you outside the root
# working directory). For each type of file, create a subdirectory in the output directory.
# The name should be the type of file. You can determine the type of the file from its
# extension. If there is no extension, move it to a subdirectory named “others”. You can
# ignore the duplicate files (files with identical names)
rm -rf "./output_dir"
mkdir -p "./output_dir"
extensions="`find $directory -type f -name '*.*' -not \(${skip_files} \) | sed 's|.*\.||'`"

for extension in $extensions; do
    mkdir -p ./output_dir/$extension
    touch ./output_dir/$extension/desc_"$extension".txt
done

# Recursively, from the root working directory and all of its sub-directories, collect the files
# whose types are not in the input file.

find $directory -type f -not \($skip_files \) | while read file; do
    filename=$(basename -- "$file") #delete directory
    extension="${filename##*.}" #longest matching pattern deleted
    filename="${filename%.*}" #from the end, delete shortest matched pattern
    

    # For each type of file, you also need to create a text file in the subdirectory of that type. In
    # this file, you need to add the relative paths(path from the root working directory) of all
    # the files of that type. Do this for the other types of files as well

    if [ -d "./output_dir/$extension" ];then
        cp "$file" "./output_dir/$extension"
        echo "${file/%."$extension"/}"|cut -c 3-  >> ./output_dir/$extension/desc_"$extension".txt
    else
        mkdir -p "./output_dir/others"
        touch "./output_dir/others/desc_others.txt"
        cp "$file" "./output_dir/others"
        echo "$file"|cut -c 3- >> ./output_dir/others/desc_others.txt
    fi
done

echo "type_of_file" > ext.csv
ls ./output_dir/ >> ext.csv
echo "ignored files" >> ext.csv


echo "no_of_files" > count.csv

## uncomment following block to count duplicates
# for dir in ./output_dir/*/     
# do
#     wc -l < "$dir"desc_*.txt >> count.csv
# done

## without duplicates
for dir in ./output_dir/*/     
do
    count=`ls ${dir} | wc -l`
    count=$((count-1))
    echo "$count" >> count.csv
done

count=0
while read i; do
    count_i=0
    # i=${i//[$' \t\r\n']}
    count_i=`find "$directory" -type f -name *.${i} | wc -l`
    count=$((count+count_i))
    echo "$count"
done < $2

echo "$count" >> count.csv


paste -d , ext.csv count.csv > output.csv
rm ext.csv
rm count.csv
    