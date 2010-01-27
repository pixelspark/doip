#!/bin/bash

# Incrementally update the Dutch XIB from the English XIB
ibtool --generate-stringsfile ./English.lproj/MainMenu.strings ./English.lproj/MainMenu.xib
ibtool --previous-file ./English.lproj/MainMenu.old.xib  --incremental-file ./Dutch.lproj/MainMenu.xib  --strings-file ./Dutch.lproj/MainMenu.strings --localize-incremental --write ./Dutch.lproj/MainMenu.new.xib ./English.lproj/MainMenu.xib
rm ./English.lproj/MainMenu.old.xib
cp ./English.lproj/MainMenu.xib ./English.lproj/MainMenu.old.xib

rm ./Dutch.lproj/MainMenu.xib
mv ./Dutch.lproj/MainMenu.new.xib ./Dutch.lproj/MainMenu.xib

# Integrate Dutch strings into Dutch nib again
ibtool --strings-file Dutch.lproj/MainMenu.strings --write Dutch.lproj/MainMenu.xib English.lproj/MainMenu.xib

