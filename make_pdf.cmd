rem @echo off
rem
rem ---------------------------------------------------------------
rem Builds a README.pdf from the README.md source file.
rem Assumes pandoc is installed and on the PATH.
rem
rem Norman Dunbar.
rem February 2017.
rem ---------------------------------------------------------------
rem
pandoc --from markdown --to latex --toc --toc-depth=3 --output README.pdf --variable linkcolor=Gray --variable urlcolor=Gray --variable toccolor=Gray --variable margin-left=3cm --variable margin-right=3cm --variable margin-top=3cm --variable margin-bottom=4cm README.md