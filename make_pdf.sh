#! /usr/env bash
#---------------------------------------------------------------
# Builds a README.pdf from the README.md source file.
# Assumes pandoc is installed and on the PATH.
# Norman Dunbar.
# February 2017.
#---------------------------------------------------------------
pandoc --from markdown --to latex --toc --toc-depth=3 --output README.pdf --variable linkcolor=Gray --variable urlcolor=Gray --variable toccolor=Gray --variable margin-left=3cm --variable margin-right=3cm --variable margin-top=3cm --variable margin-bottom=4cm README.md