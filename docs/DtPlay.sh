#!/bin/sh

if ! [ -x "$(command -v DtPlay)" ]; then
  echo "Error: DtPlay is not installed."
  echo "Install using Linux SDK and DtPlay packages from DekTec website."
  echo "https://www.dektec.com/downloads/SDK/"
  exit 1
fi

if [ -z "$1" ]; then
  echo "Error: No argument supplied."
  exit 1
fi

# canal 14
DtPlay $1 -mt ISDBT -t 215 -mf 473.14 -l 0
