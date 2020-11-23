#!/bin/bash
# runs on output of lstorage -R > [file]
# usage : convertToPaths.sh [file]
# writes output to [file].withdir
cat  $1 | awk '{ if ( $1 ~ "proj" ) dir=$1; else print  dir"/"$1;}' | sed 's/://' | grep '.root' > $1.withdir
