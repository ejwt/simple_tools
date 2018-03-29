@echo off

rem --------------------------------------------------------------------------
rem   This batch files simply removes all CVS directories from the current
rem   directory and any sub-directories.  Use this batch file when you want
rem   to send off the source code but do not want to include all of the CVS
rem   overhead files.
rem --------------------------------------------------------------------------

for /r %%v in (CVS) do rmdir /S /Q %%v


