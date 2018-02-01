#!/bin/sh

if ! [ -x "$(command -v DtPlay)" ]; then
  echo "Error: DtPlay is not installed."
  exit 1
fi

if [ -z "$1" ]; then
  echo "Error: No argument supplied."
  exit 1
fi


# canal 20
DtPlay $1 -mt ISDBT -t 215 -mf 509.14 -l 0
