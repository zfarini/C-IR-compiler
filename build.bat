@echo off

REM -d1reportTime
set CommonCompilerFlags=-diagnostics:column -WL -Zi -nologo -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -wd4244 -FC

set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib kernel32.lib



cl %CommonCompilerFlags% main.c /link %CommonLinkerFlags%
