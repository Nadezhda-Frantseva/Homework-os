#!/bin/bash
if [[ $2 = "top_places" ]]; then
    if [ ! $# -eq 5 ]; then
        echo "There must be five arguments !"
        exit 1
    fi
    if [ ! -f $1 ]; then
        echo "The first argument must be a file !"
        exit 2
    fi
    if [ ! $(echo "$3" | egrep "^[A-Z][0-9]+$") ]; then
        echo "The third argument must be in the format of a category (capital letter followed by numbers) !"
        exit 3
    fi
    if [ ! $(echo "$4" | egrep "^[0-9]+$") ]; then
        echo "The fourth argument must be a number !"
        exit 4
    fi
    if [ ! $(echo "$5" | egrep "^[0-9]+$") ]; then
        echo "The fifth argument must be a number !"
        exit 5
    fi
    result=""
    result=$(cat $1 | egrep "$3")
    temp=$4
    newresult=""
    while read line; do
        if [ $(echo "$line" | cut -d ":" -f3 | egrep "[[:digit:]]+") ] && [ $(echo "$line" | cut -d ":" -f3) -le $temp ]; then 
            newresult=$newresult$'\n'$(echo "$line" | cut -d: -f4)
        fi
    done < <(echo "$result")
    echo "$newresult" | tail -n +2 | sort | uniq -c | sort -rn -k1 | head -n $5 | tr -s ' '
elif [[ $2 = "parts" ]]; then
    if [ ! $# -eq 3 ]; then
        echo "There must be three arguments !"
        exit 1
    fi
    if [ ! -f $1 ]; then
        echo "The first argument must be a file !"
        exit 2
    fi
    if [[ -z $3 || $(echo "$3" | egrep "^[0-9]+$") ]]; then
        echo "The second argument must be name !"
        exit 3
    fi
    result2=""
    result2=$(cat $1 | egrep "$3" | cut -d: -f1,2 | awk -F: '{print $2" "$1}' | sort | uniq)
    while read line; do
        startRepeat=""
        endRepeat=""
        startRepeat=$(echo "$result2" | egrep -n "$(echo "$line" | cut -d" " -f1 )" | cut -d: -f1 | head -n 1)
        endRepeat=$(echo "$result2" | egrep -n "$(echo "$line" | cut -d" " -f1 )" | cut -d: -f1 | tail -n 1)
        if [[ $startRepeat -eq $endRepeat ]]; then
            echo "$line"
            continue
        fi
        diff=""
        diff=$(($endRepeat-$startRepeat+1))
        codе=""
        code=$(echo "$result2" | head -n $endRepeat | tail -n $diff | sort -n -t"." -k3 -k2 -k1)
        echo -n "$(echo "$code" | head -n 1 | cut -d" " -f1)" 
        counter=0
        while read line1; do
        counter=$(($counter+1))
            if [ $counter -eq $diff ]; then
                echo -n " $(echo "$line1" | cut -d" " -f2)"
            else
                echo -n " $(echo "$line1" | cut -d" " -f2), "
            fi
        done < <(echo "$code")
        echo
        if [ $(echo "$result2" | wc -l) -eq $endRepeat ]; then 
            break
        fi
    done < <(echo "$result2")
else 
    echo "The second argument must be a subcommand name (top_places or parts) !"
    exit 1
fi