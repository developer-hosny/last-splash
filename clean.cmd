@echo off

rem Delete the folders "Binaries", "Intermediate", and "Saved"
rem and all of their subfolders.

rmdir /s /q Binaries Intermediate Saved

rem Pause the batch file so that you can see the results.

pause