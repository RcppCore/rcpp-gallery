#!/bin/bash
infile=${1}
## echo $infile

p2root=${2}
echo $p2root

bfn=$(basename $infile)
## echo $bfn

nodate=$(echo ${bfn} | sed -r 's/[0-9]{4}-[0-9]{2}-[0-9]{2}-//')
## echo $nodate

slug=$(echo ${nodate} | sed -r 's/-fig.*//')
## echo $slug

target=${p2root}/_site/articles/${slug}
## echo $target

if [ ${#slug} -ge 0 ] ;
then
    ## target dir exists
    if [ -d "${target}" ] ;
    then
        echo "Writing ${1} to ${target}/figure/"
        rsync -rv ${infile} ${target}/figure/
        echo ${target}/figure
        ls ${target}/figure
    else
        echo "Skipping ${1}: ${target}/figure not writeable"
    fi
else
    echo "Skipping ${1}: empty slug"
fi
