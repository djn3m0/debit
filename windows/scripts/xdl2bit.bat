:: run xdl2bit with the correct arguments
:: this script is used as proxy in order not to clobber the registry
:: with complex commands for file association

@ECHO OFF

SET debitpath=%~dp0%
CD %debitpath%

bin\xdl2bit.exe -d "%debitpath%data" -i %1 --bramdump -b "%~pn1%_debit.bit"
