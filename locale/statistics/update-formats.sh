#!/bin/bash

FFMPEG=ffmpeg

do_collect()
{
    tn=0
    tr=-1
    $FFMPEG -formats 2>/dev/null | while read line; do
		if [ "$line" = "--" ]; then
            tn=1
            tr=0
            echo "<table class=\"ftable\">"
			echo "<thead>"
			echo "<tr><th>format</th><th>name</th><th>format</th><th>name</th><th>format</th><th>name</th></tr>"
			echo "</thead>"
			echo "<tbody>"
        elif [ -z "$line" ]; then
            if [ "$tr" -ne 3 ]; then
                echo "</tr>"
            fi
            tn=2
            tr=-1
			echo "</tbody>"
            echo "</talbe>"
        fi

	    cell=''
	    if [ "$tn" -eq 1 ]; then
			arr=($line)
			formatType=${arr[0]}
			shortName=${arr[1]}
			longName=${arr[@]:2}
	    	if [ "$formatType" != "E" ] && [ -n "$shortName" ] && [ -n "$longName" ]; then
				if [ "$shortName" = "asf" ] || [ "$shortName" = "avi" ] || [ "$shortName" = "dv" ] \
						|| [ "$shortName" = "flv" ] || [ "$shortName" = "matroska" ] || [ "$shortName" = "h264" ] \
						|| [ "$shortName" = "mov,mp4,m4a,3gp,3g2,mj2" ] || [ "$shortName" = "mpeg" ] \
						|| [ "$shortName" = "ogg" ] || [ "$shortName" = "wav" ]; then
	                cell="<td><b>$longName<b></td>"
				else
					cell="<td>$longName</td>"
				fi
            fi
        fi
        if [ -n "$cell" ]; then
            if [ "$tr" -eq 0 ]; then
				echo "<tr>"
			fi
			echo $cell
			let tr+=1
			if [ "$tr" -eq 4 ]; then
				tr=0
				echo "</tr>"
			fi
		fi
	done
}

LANG=en
do_collect >__formats.php

echo "Starting file transfer to host..."
#sftp -b formats-update.batch `cat host.cfg` 
echo "Transfer complete."
