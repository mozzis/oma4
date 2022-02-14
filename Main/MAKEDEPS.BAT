@rem echo off
rem file formdeps.bat -- update dependencies in the oma4000 makefile
rem -M causes project.gen to be used -- dependencies for all oma4000 obj's
rem after makedeps, use uniqdep to remove duplicates from the dependencies
j:\make\makedeps -Foma4000.dep -M
copy oma4000.dep %tmp%
j:\make\uniqdep < %tmp%oma4000.dep > oma4000.dep
