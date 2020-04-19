#!/bin/bash
#时间
date=`date +"%Y-%m-%d__%H:%M:%S"`
#名字
UserName=`hostname`
#OS版本
UserOS=(`cat /etc/issue | awk '{printf("%s %s %s\n", $1,$2,$3)}'`)
#内核
NHe=(`uname -a | awk '{printf("%s", $3)}'`)
#启动时间
Uptime=(`uptime -p |tr -s " " "_"`)
#平均负载
load=`cat /proc/loadavg | cut -d ' ' -f 1-3`

Disk=(`df -m | grep ^/dev | awk  '{printf("%s %s\n",$2, $4)}'`)
Diskuse=0
DiskFree=0
for ((i = 0; i < ${#Disk[@]}; i+=2)) do
    ((Diskuse+=${Disk[i]}))
    ((DiskFree+=${Disk[i+1]}))
done
#磁盘总量, 磁盘空闲
#磁盘已利用率
a=$((100-$DiskFree*100/$Diskuse))
#内存大小
neicun=(`free -m | head -n 2 | tail -n 1 | awk '{printf("%d %d"), $3,$2}'`)
#内存已用
NClv=`echo "scale=1;${neicun[0]}*100/${neicun[1]}" | bc`


#cpu温度
cpuTemp=`cat /sys/class//thermal/thermal_zone0/temp`
cpuTemp=`echo "scale=2;$cpuTemp/1000" | bc`



if [[ `echo $cpuTemp '>=' 70 | bc -l` == 1 ]];then
        warn3="warning"
elif [[ `echo $cpuTemp '>=' 50 | bc -l` == 1 ]];then
        warn3="note"
else
        warn3="normal"
fi

if [[ `echo $NClv '>=' 80 | bc -l` == 1 ]];then
        warn2="warning"
elif [[ `echo $NClv '>=' 70 | bc -l` == 1 ]];then
        warn2="note"
else
        warn2="normal"
fi
if [[ `echo $a '>=' 90 | bc -l` == 1 ]];then
        warn1="warning"
elif [[ `echo $a '>=' 80 | bc -l` == 1 ]];then
        warn1="note"
else
        warn1="normal"
fi

echo "${date} ${UserName} ${UserOS[0]}_${UserOS[1]}_${UserOS[2]} ${NHe} ${Uptime} ${load} ${Diskuse} ${a}% ${neicun[1]} ${NClv}% ${cpuTemp} ${warn1} ${warn2} ${warn3}"
