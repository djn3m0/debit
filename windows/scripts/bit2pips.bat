:: run bit2pdf with the correct arguments
:: this script is used as proxy in order not to clobber the registry
:: with complex commands for file association

@ECHO OFF

SET debitpath=%~dp0%
CD %debitpath%

bin\debit.exe -d "%debitpath%data" -i %1 --pipdump > "%~pn1%_pips.txt"
