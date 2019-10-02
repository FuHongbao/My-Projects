#!/bin/bash
nowtime=`date "+%Y-%m-%d__%H:%M:%S"`
cpu_load=`cat /proc/loadavg | cut -d ' ' -f 1-3`
temp=(`cat /sys/class/thermal/thermal_zone0/temp | awk '{printf("%.2f℃", $1 / 1000)}'`)
eval `head -n 1 /proc/stat | awk -v sum1=0 -v idle1=0 '{for (i=2; i<=8; i++) {sum1+=$i} printf("sum1=%.0f;idle1=%.0f", sum1, $5)}'`
sleep 0.5
eval `head -n 1 /proc/stat | awk -v sum2=0 -v idle2=0 '{for (i=2; i<=8; i++) {sum2+=$i} printf("sum2=%.0f;idle2=%.0f", sum2, $5)}'`
cpu_use=`echo "scale=4;(1-($idle2-$idle1)/($sum2-$sum1))*100" | bc`
cpu_use=`printf "%.2f" $cpu_use`
warnLevel="normal"
if [[ `echo $cpu_use '>=' 70 | bc -l` == 1 ]];then
    warnLevel="warning"
elif [[ `echo $cpu_use '>=' 50 |bc -l` == 1 ]];then
    warnLevel="note"
fi
echo "${nowtime} ${cpu_load} ${cpu_use} ${temp} ${warnLevel}"




