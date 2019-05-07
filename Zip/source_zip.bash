#!/bin/bash

# This script prepares the source code zip file for publication on the web
# Must be used on a clean checkout of the Release version of FEBio

if [ $# == 0 ]; then
	echo "Usage: source_zip.bash version (e.g. source_zip.bash 2.7.1.11033)"
	exit
fi

if [ -d FEBio2 ]; then
    rm -rf FEBio2
fi

mkdir FEBio2
mkdir FEBio2/doc

cp ../Documentation/README.txt FEBio2/doc
cp ../Documentation/FEBio_EULA.pdf FEBio2/doc
cp -r ../build FEBio2
cp -r ../FECore FEBio2
cp -r ../NumCore FEBio2
cp -r ../FEBioHeat FEBio2
cp -r ../FEBioMech FEBio2
cp -r ../FEBioMix FEBio2
cp -r ../FEBioOpt FEBio2
cp -r ../FEBioPlot FEBio2
cp -r ../FEBio2 FEBio2
cp -r ../FEBioFluid FEBio2
cp -r ../FEBioXML FEBio2
cp -r ../FEBioTest FEBio2
cp -r ../FEBioLib FEBio2
cp -r ../VS2013 FEBio2
cp -r ../VS2015 FEBio2

zip -r febiosource_${1}.zip FEBio2
