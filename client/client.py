#!/usr/bin/env python
# -*- coding: UTF-8 -*- 
#import socket
#import select
import ftplib

host='166.111.82.233'
flag=1

username='anonymous'    #这里直接弄匿名登陆吧
password='hello'        #匿名这里随便输入
                        #可以使用自己的字典对这里进行username和password的改进

ftp = ftplib.FTP()
ftp.set_debuglevel(2) #打开调试级别2，显示详细信息
ftp.connect(host, 6789) #连接的ftp sever和端口 
ftp.login()
print("hello")
print(ftp.sendcmd('SYST'))
#print(ftp.getwelcome())
