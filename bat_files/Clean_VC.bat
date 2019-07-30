IF "%OS%" == "Windows_NT" GOTO WinNT
FOR %%i IN (Debug, Release) DO DELTREE %%i
GOTO CONT2

:WinNT
FOR %%i IN (Debug, Release) DO IF EXIST %%i RD %%i /S/Q

:CONT2
FOR %%i IN (PLG, APS, NCB) DO IF EXIST *.%%i DEL *.%%i /F
