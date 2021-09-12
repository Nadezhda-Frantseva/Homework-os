#!/bin/bash
if [ ! $# -eq 1 ]; then
    echo "There must be only one argument !"
    exit 1
fi
if (test -f $1); then
    mktemp -d "temp_data_txt.XXXXX"
    while read line1; do
        if [ $(echo "$line1" | egrep -v "^#") ]; then
            date=""
            date=$(curl "$line1" 2>/dev/null | egrep -o "[[:digit:]]{1,2}.[[:digit:]]{2}.[[:digit:]]{4}" | awk -F. '{if(length($1)==1)print "0"$0; else print $0}')
            while read line2; do
                catline=""
                catline=$(egrep -n "<tr class=\"Head1 NewPage H1" | sed 's/<tr class=\"Head1 NewPage H1//g' | cut -d '"' -f1)
                while read line3; do
                    category=""
                    start=""
                    end=""
                    category=$(echo "$line3" | cut -d: -f2)
                    start=$(echo "$line3" | cut -d: -f1)
                    if [ $(echo "$catline" | wc -l) -eq 1 ]; then
                        end=$(curl --silent "$line1" 2>/dev/null | egrep -n "</body>" | cut -d: -f1)
                    else 
                        catline=$(echo "$catline" | tail -n +2) 
                        end=$(echo "$catline" | head -n 1 |cut -d: -f1)
                    fi
                    diff=""
                    diff=$(($end-$start))
                    code=""
                    code=$(curl --silent "$line1" 2>/dev/null | head -n $end | tail -n $diff | egrep "^<tr class=\"ResLine" | sed  's/<[^>]*>/+/g' | tr -s '+')
                    while read line4; do
                        place=""
                        place=$(echo "$line4" | cut -d+ -f2 | cut -d. -f1)
                        name=""
                        name=$(echo "$line4" | cut -d+ -f3 | sed 's/, /,/g' | awk -F ',' '{print $2, $1}' | cut -d, -f1 | sed 's/^ //g')
                        national=""
                        national=$(echo "$line4" | cut -d+ -f4)
                        call=""
                        call=$(echo "$line4" | cut -d+ -f5 | awk '{if($1~"^&nbsp;$")print"";else print $1}')
                        minsec=""
                        minsec=$(echo "$line4" | cut -d+ -f6)
                        fox=""
                        fox=$(echo "$line4" | cut -d+ -f7 | awk '{if($1~"^&nbsp;$")print"";else print $1}')
                        strnum=""
                        strnum=$(echo "$line4" | cut -d+ -f8)
                        echo "$date:$category:$place:$name:$national:$call:$minsec:$fox:$strnum"
                        echo "$date:$category:$place:$name:$national:$call:$minsec:$fox:$strnum" 2>/dev/null >>/tmp/temp_data_txt.XXXXX
                    done < <(echo "$code")
                done < <(echo "$catline")
            done < <(curl --silent $line1)
        fi
    done < $1
else
    echo "The argument must be a file !"
    exit 2
fi