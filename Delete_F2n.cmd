@echo off
set /a n=%1
if %n% LSS 0 (
    echo N must be a non-negative integer.
    exit /b
)
for /L %%i in (0, 2, %n%) do (
    if exist "F%%i.Dat" (
        del "F%%i.Dat"
        echo Deleted F%%i.Dat
    ) else (
        echo F%%i.Dat not found
    )
)
echo Done
