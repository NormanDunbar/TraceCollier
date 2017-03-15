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
pandoc --from markdown --to latex --toc --toc-depth=3 --output README.pdf --toc --toc-depth=3 --listings -H listings_setup.tex --variable fontfamily="utopia" --variable linkcolor="Cool Grey" --variable urlcolor="Cool Grey" --variable toccolor="Cool Grey" --variable margin-left=3cm --variable margin-right=3cm --variable margin-top=3cm --variable margin-bottom=4cm README.md