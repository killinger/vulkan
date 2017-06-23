@echo off

SETLOCAL
set SourceFile=test.c
set SourceDirectory=test
set OutputDirectory=.\Debug\

cls
pushd %OutputDirectory%
cl -W4 /LD /nologo ..\%SourceDirectory%\%SourceFile%
popd