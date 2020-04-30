#!/bin/bash
date=`date +"%Y-%m-%d__%H:%M:%S"`

XiaoDisk=(`df -m | grep ^/dev | awk -v k=$date '{printf("%s 1 %s %s %s %s\n",k,$6,$2,$4,$5)}'`)

Disknum=`df -m | grep ^/dev | wc -l`
Diskuse=0
Diskfree=0

for ((i = 0; i < ${#XiaoDisk[@]}; i+=6)) do
        echo ${XiaoDisk[i]} ${XiaoDisk[i+1]} ${XiaoDisk[i+2]} ${XiaoDisk[i+3]} ${XiaoDisk[i+4]} ${XiaoDisk[i+5]}
    ((Diskuse+=${XiaoDisk[i+3]}))
    ((Diskfree+=${XiaoDisk[i+4]}))
done
a=$Diskfree
b=$Diskuse

echo $date "0" "disk" $Diskuse $Diskfree `echo "100-$a*100/$b" |bc`%
