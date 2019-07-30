@echo off

rem --------------------------------------------------------------------------
rem   This batch file is intended to remove all the files for Source Insight. 
rem   必须小心!!  因为此批处理文件是递归删除!!! 
rem   
rem   某些扩展名并不是Source Insight专用，例如，*.PS还可能是Acrobat文档!!!
rem --------------------------------------------------------------------------

del /S /Q *.IMB
del /S /Q *.IMD
del /S /Q *.IAB
del /S /Q *.IAD
del /S /Q *.PR
rem del /S /Q *.PS
del /S /Q *.PFI
rem del /S /Q *.PO
del /S /Q *.PRI
del /S /Q *.WK3
del /S /Q *.SearchResults
del /S /Q *.CF3

