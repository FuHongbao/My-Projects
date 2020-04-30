#!/bin/bash

NowTime=`date +%Y-%m-%d__%H:%M:%S`

info=`ps -aux | awk '{if($3>=4.0 || $4>=4.0)print $2}'`

if [[ $info != "" ]];then
    sleep 5
else
    exit 0
fi
info=`ps -aux | awk '{if($3>=4.0 || $4>=4.0) print $2}'`

if [[ $info != "" ]];then
    for i in $info;do
        eval $(ps -p $i -u | awk -v pid=0 -v name="" -v usr="" -v cpunum=0 -v memnum=0 '{if($3>=4.0 || $4>=4.0) printf ("name=%s; usr=%s; pid=%d; cpunum=%.2f; memnum=%.2f;",$11, $1, $2, $3, $4)}')
        if [[ $pid -gt 0 ]];then
            echo "$NowTime $name $pid $usr $cpunum% $memnum%"
        fi
  done
fi

