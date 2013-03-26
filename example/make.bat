::select working directory 
@SET DIR_WORKING=.

::select a directory, containing <varname>.c & <varname>.h template files
@SET DIR_TEMPLATES=.\templates

::default build paths
@SET DIR_TCC=..\tcc\
@SET PATH_TCC=%DIR_TCC%\tcc.exe
@SET DIR_SRC_CGETSET=..\src\
@SET PATH_BIN_CGETSET=.\cgetset.exe

::debug report
@ECHO DIR_TCC: %DIR_TCC%
@ECHO PATH_TCC: %PATH_TCC%
@ECHO DIR_SRC_CGETSET: %DIR_SRC_CGETSET%
@ECHO PATH_BIN_CGETSET: %PATH_BIN_CGETSET%
@PAUSE

::ensure only newly compiled binary is executed
DEL %PATH_BIN_CGETSET%

::build cgetset application
@ECHO:
@ECHO Build
%PATH_TCC% -I%DIR_TCC%\Include %DIR_SRC_CGETSET%\cgetset.c -L%DIR_TCC%\lib

::execute the build binary
@ECHO:
@ECHO Execute
%PATH_BIN_CGETSET% example.txt templates

@ECHO:
@PAUSE
