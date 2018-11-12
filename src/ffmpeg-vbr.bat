@ECHO off
SET FF_DRIVE=%~d0
SET FF_PATH=%~p0
%FF_DRIVE%
CD "%FF_PATH%"

FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==Matrix_High SET Matrix_High=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==Matrix_Medium SET Matrix_Medium=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==Matrix_Low SET Matrix_Low=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==VBR_threshold SET /A VBR_threshold=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==twopass_threshold SET /A twopass_threshold=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==HQ_threshold SET /A HQ_threshold=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==XHQ_threshold SET /A XHQ_threshold=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==MediumBitrate_threshold SET /A MediumBitrate_threshold=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==LowBitrate_threshold SET /A LowBitrate_threshold=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==HQ_params SET HQ_params=%%b
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==Interlaced SET Interlaced=%%b


SET ORIGINAL=%*
IF "%Interlaced%"=="1" SET ORIGINAL=%ORIGINAL: -f mpeg2video = -flags:v:0 +ilme+ildct -alternate_scan:v:0 1 -f mpeg2video %
IF "%Interlaced%"=="2" SET ORIGINAL=%ORIGINAL: -f mpeg2video = -vf bwdif=mode=0 -f mpeg2video %
REM IF "%Interlaced%"=="2" SET ORIGINAL=%ORIGINAL: -f mpeg2video = -vf yadif -f mpeg2video %
SET bitrate=
SET size=
SET HQ=NO
SET height=
SET CQM=
SET maxrate=8500000


REM -----------------------------------------------------------------------------------------------------------------------------------------------------


:forcemode
SET force_mode=%1
SET mode_value=%2
IF NOT [%force_mode%==[-mode GOTO assemble
IF %2==cbr SET /A VBR_threshold=0 & SET /A twopass_threshold=0 & SET /A HQ_threshold=0 & SET /A XHQ_threshold=0 & GOTO assemble
IF %2==vbr1pass SET /A VBR_threshold=1 & SET /A twopass_threshold=0 & SET /A HQ_threshold=0 & SET /A XHQ_threshold=0 & GOTO assemble
IF %2==vbr2pass SET /A VBR_threshold=1 & SET /A twopass_threshold=1 & SET /A HQ_threshold=0 & SET /A XHQ_threshold=0 & GOTO assemble
IF %2==cbr_hq SET /A VBR_threshold=0 & SET /A twopass_threshold=0 & SET /A HQ_threshold=1 & SET /A XHQ_threshold=0 & GOTO assemble
IF %2==vbr1pass_hq SET /A VBR_threshold=1 & SET /A twopass_threshold=0 & SET /A HQ_threshold=1 & SET /A XHQ_threshold=0 & GOTO assemble
IF %2==vbr2pass_hq SET /A VBR_threshold=1 & SET /A twopass_threshold=1 & SET /A HQ_threshold=1 & SET /A XHQ_threshold=0 & GOTO assemble
IF %2==vbr2pass_xhq SET /A VBR_threshold=1 & SET /A twopass_threshold=1 & SET /A HQ_threshold=1 & SET /A XHQ_threshold=1

:assemble
IF [%1==[ GOTO branch
IF %1==-b:v:0 GOTO bitrate
IF %1==-s GOTO size
IF "[%~x1"=="[.vob" GOTO out
IF "[%~x1"=="[.m2v" SET out_m2v=%1 & SET out_path=%~dp1
IF %1==-i GOTO infile
SHIFT
GOTO assemble

:bitrate
SHIFT
SET /A bitrate=%1
SHIFT
GOTO assemble

:size
SHIFT
set size=%1
SHIFT
GOTO assemble

:out
SET out=%1
SET out_path=%~dp1
SHIFT
GOTO assemble

:infile
SHIFT
SET infile=%1
SET infile=%infile:"=%
SHIFT
GOTO assemble



REM -----------------------------------------------------------------------------------------------------------------------------------------------------


:branch
SET ORIGINAL=%ORIGINAL:!=_$$§§$$_%
IF DEFINED out SET out=%out:!=_$$§§$$_%
IF DEFINED out_path SET out_path=%out_path:!=_$$§§$$_%
IF DEFINED out_m2v SET out_m2v=%out_m2v:!=_$$§§$$_%
SETLOCAL EnableDelayedExpansion
IF [%force_mode%==[-mode SET ORIGINAL=!ORIGINAL:%force_mode% %mode_value% =!

IF %VBR_threshold%==1 SET /A VBR_threshold=9000
IF %HQ_threshold%==1 SET /A HQ_threshold=9000
IF %XHQ_threshold%==1 SET /A XHQ_threshold=9000
IF %twopass_threshold%==1 SET /A twopass_threshold=9000

ECHO =============================================================================
ECHO. 
ECHO FFMpeg-VBR Plugin Settings:
ECHO -----------------------------------------------------
ECHO VBR Threshold:               %VBR_threshold% kbps
ECHO 2-Pass Threshold:            %twopass_threshold% kbps
ECHO HQ Threshold:                %HQ_threshold% kbps
ECHO Extreme HQ Threshold:        %XHQ_threshold% kbps
ECHO Medium Bitrate Threshold:    %MediumBitrate_threshold% kbps
ECHO Low Bitrate Threshold:       %LowBitrate_threshold% kbps
ECHO -----------------------------------------------------
ECHO. 

IF [%bitrate%==[ GOTO dont_touch
SET /A VBR_threshold=%VBR_threshold% * 1000
SET /A HQ_threshold=%HQ_threshold% * 1000
SET /A XHQ_threshold=%XHQ_threshold% * 1000
SET /A twopass_threshold=%twopass_threshold% * 1000
SET /A MediumBitrate_threshold=%MediumBitrate_threshold% * 1000
SET /A LowBitrate_threshold=%LowBitrate_threshold% * 1000
IF EXIST "%Matrix_High%.txt" SET /P CQM=<"%Matrix_High%.txt" & SET Matrix=%Matrix_High%
IF %bitrate% GTR 8000000 SET maxrate=9500000
IF %bitrate% LSS %MediumBitrate_threshold% IF EXIST "%Matrix_Medium%.txt" SET /P CQM=<"%Matrix_Medium%.txt" & SET Matrix=%Matrix_Medium%
IF %bitrate% LSS %LowBitrate_threshold% IF EXIST "%Matrix_Low%.txt" SET /P CQM=<"%Matrix_Low%.txt" & SET Matrix=%Matrix_Low%
IF %bitrate% LEQ %twopass_threshold% GOTO 2pass
IF %bitrate% GTR %VBR_threshold% GOTO CBR


REM -----------------------------------------------------------------------------------------------------------------------------------------------------


:1pass_VBR
SET mode=1-pass VBR
SET ORIGINAL=!ORIGINAL: -g 15 = -g 12 !
SET ORIGINAL=!ORIGINAL: -g 18 = -g 12 !
SET ORIGINAL=!ORIGINAL:-maxrate:v:0 %bitrate%=-maxrate:v:0 %maxrate% -dc 10 -bf 2 -qmin 1 -lmin 0.75 -mblmin 50!
SET ORIGINAL=!ORIGINAL:-minrate:v:0 %bitrate% =!
SET ORIGINAL=!ORIGINAL:-c:v:0 mpeg2video=-c:v:0 mpeg2video %CQM%!
SET HQ_params=%HQ_params:-bf 2=%
IF %bitrate% LEQ %HQ_threshold% SET ORIGINAL=!ORIGINAL:-c:v:0 mpeg2video=-c:v:0 mpeg2video %HQ_params%! & SET HQ=YES
IF "%HQ%"=="NO" IF %bitrate% LEQ %XHQ_threshold% SET ORIGINAL=!ORIGINAL:-c:v:0 mpeg2video=-c:v:0 mpeg2video %HQ_params%! & SET HQ=YES
GOTO dont_touch


:CBR
SET mode=CBR
SET Fixed_Rate=
SET height=%size:~-3%
IF %height%==288 IF %bitrate% GTR 7190000 SET Fixed_Rate=7190000
IF %height%==240 IF %bitrate% GTR 6330000 SET Fixed_Rate=6330000
IF "%Fixed_Rate%"=="" GOTO NoFix
SET ORIGINAL=!ORIGINAL:-b:v:0 %bitrate%=-b:v:0 %Fixed_Rate%!
SET ORIGINAL=!ORIGINAL:minrate:v:0 %bitrate%=minrate:v:0 %Fixed_Rate%!
SET ORIGINAL=!ORIGINAL:maxrate:v:0 %bitrate%=maxrate:v:0 %Fixed_Rate%!

:NoFix
SET ORIGINAL=!ORIGINAL:-c:v:0 mpeg2video=-c:v:0 mpeg2video -dc 10 %CQM%!
IF %bitrate% LEQ %HQ_threshold% SET ORIGINAL=!ORIGINAL:-c:v:0 mpeg2video=-c:v:0 mpeg2video %HQ_params%! & SET HQ=YES
IF "%HQ%"=="NO" IF %bitrate% LEQ %XHQ_threshold% SET ORIGINAL=!ORIGINAL:-c:v:0 mpeg2video=-c:v:0 mpeg2video %HQ_params%! & SET HQ=YES

:dont_touch
setlocal DisableDelayedExpansion
SET ORIGINAL=%ORIGINAL:_$$§§$$_=!%
ECHO ^<%date%  %time%^> 
ECHO File Name: %infile%
ECHO Encoding Mode: %mode%   HQ: %HQ%   Custom Quant Matrix: %Matrix%
ECHO. 
ECHO Executing command generated by the ffmpeg-VBR plugin:
ECHO ffmpeg.exe %ORIGINAL%
ECHO. 
ffmpeg.exe %ORIGINAL%
ECHO. 
GOTO :EOF


:2pass
SET mode=2-pass VBR
SET FIRST=%ORIGINAL%
SET FIRST=!FIRST: -g 15 = -g 12 !
SET FIRST=!FIRST: -g 18 = -g 12 !
SET FIRST=!FIRST:-maxrate:v:0 %bitrate%=-maxrate:v:0 %maxrate% -dc 10 -bf 2 -q:v 2!
SET FIRST=!FIRST:-minrate:v:0 %bitrate% =!
IF %bitrate% LEQ %XHQ_threshold% IF EXIST "Fox New.txt" SET /P CQM=<"Fox New.txt" & SET Matrix=Fox New
IF %bitrate% LEQ %HQ_threshold% SET HQ_params=-b_strategy 2 -brd_scale 2 -profile:v 4
IF %bitrate% LEQ %XHQ_threshold% SET HQ_params=-b_strategy 2 -brd_scale 2 -profile:v 4
SET FIRST=!FIRST:-c:v:0 mpeg2video=-c:v:0 mpeg2video %CQM%!
IF %bitrate% LEQ %HQ_threshold% SET FIRST=!FIRST:-c:v:0 mpeg2video=-c:v:0 mpeg2video %HQ_params%! & SET HQ=YES
IF "%HQ%"=="NO" IF %bitrate% LEQ %XHQ_threshold% SET FIRST=!FIRST:-c:v:0 mpeg2video=-c:v:0 mpeg2video %HQ_params%!
IF %bitrate% LEQ %XHQ_threshold% SET HQ=YES (extreme)
IF NOT DEFINED out_m2v SET FIRST=!FIRST:%out%=-an -passlogfile "%out_path%ffmpeg" -pass 1 -y "NUL.avi"! & GOTO second
SET FIRST=!FIRST:%out_m2v%=-an -passlogfile "%out_path%ffmpeg" -pass 1 -y "NUL.avi"!
SET /A pos=0
:pos_loop
SET /A pos=%pos%+1
SET XYZ=!FIRST:~%pos%,7!
IF NOT [!XYZ!==[NUL.avi GOTO pos_loop
SET /A pos=%pos%+8
SET FIRST=!FIRST:~0,%pos%!

:second
SET SECOND=%ORIGINAL%
SET SECOND=!SECOND: -g 15 = -g 12 !
SET SECOND=!SECOND: -g 18 = -g 12 !
SET SECOND=!SECOND:-maxrate:v:0 %bitrate%=-maxrate:v:0 %maxrate% -dc 10 -bf 2 -lmin 0.75 -mblmin 50 -qmin 1!
SET SECOND=!SECOND:-minrate:v:0 %bitrate% =!
FOR /F "tokens=1,2 delims==" %%a IN (ff_vbr.ini) DO IF %%a==HQ_params SET HQ_params=%%b
SET HQ_params=%HQ_params:-bf 2=%
IF %bitrate% LEQ %XHQ_threshold% SET HQ_params=-pre_dia_size 5 -dia_size 5 -qcomp 0.7 -qblur 0 -preme 2 -me_method dia -sc_threshold 0 -bidir_refine 4 -profile:v 4 -mbd rd -mbcmp satd -precmp satd -cmp satd -subcmp satd -skipcmp satd
SET SECOND=!SECOND:-c:v:0 mpeg2video=-c:v:0 mpeg2video %CQM%!
IF NOT "%HQ%"=="NO" SET SECOND=!SECOND:-c:v:0 mpeg2video=-c:v:0 mpeg2video %HQ_params%!
IF NOT DEFINED out_m2v SET SECOND=!SECOND:%out%=-passlogfile "%out_path%ffmpeg" -pass 2 %out%! & GOTO twopasslog
SET SECOND=!SECOND:%out_m2v%=-passlogfile "%out_path%ffmpeg" -pass 2 %out_m2v%!

:twopasslog
setlocal DisableDelayedExpansion
SET FIRST=%FIRST:_$$§§$$_=!%
SET SECOND=%SECOND:_$$§§$$_=!%
ECHO ^<%date%  %time%^> 
ECHO File Name: %infile%
ECHO Encoding Mode: %mode%   HQ: %HQ%   Custom Quant Matrix: %Matrix%
ECHO. 
ECHO Executing first pass command generated by the ffmpeg-VBR plugin:
ECHO ffmpeg.exe %FIRST%
ECHO. 
ffmpeg.exe %FIRST%
ECHO. 
ECHO Executing second pass command generated by the ffmpeg-VBR plugin:
ECHO ffmpeg.exe %SECOND%
ECHO. 
ffmpeg.exe %SECOND%
ECHO. 
