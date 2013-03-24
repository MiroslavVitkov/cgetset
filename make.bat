@SET DIR_TCC="..\tcc"

DEL csetget.exe

@ECHO:
@ECHO Build
%DIR_TCC%\tcc.exe -I%DIR_TCC%\Include csetget.c -L%DIR_TCC%\lib

@ECHO:
@ECHO Execute
csetget.exe example.txt templates

@ECHO:
@PAUSE
