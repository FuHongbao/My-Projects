#!/bin/bash
nowTime=`date +"%Y-%m-%d__%H:%M:%S"`

UserNum=`cat /etc/passwd | grep home |grep bash | wc -l`
Users=(`cat /etc/passwd | w | tail -n 3 | cut -d ' ' -f 1`)
UserRoot=(`cat /etc/group | grep sudo | cut -d ':' -f 4- |cut -d ',' -f 1-`)
UserInfo=(`w | awk 'NR==1 {next} NR==2 {next} {print ","$1"_"$3"_"$2}' | tr -d '\n' | cut -d ',' -f 2-`) 
echo "$nowTime $UserNum [${Users[0]},${Users[1]},${Users[2]}] [${UserRoot[*]}] [${UserInfo}]"

