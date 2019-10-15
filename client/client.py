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
ftp.connect(host, 9999) #连接的ftp sever和端口 
ftp.login(user=username, passwd=password)
ftp.sendcmd('SYST')
ftp.sendcmd('TYPE I')
ftp.set_pasv(True)
filename = "test186.data"
filename1 = "out.txt"
#ftp.sendcmd("RNFR test.txt")
#ftp.sendcmd("RNTO dest/newtest.txt")

#ftp.retrbinary('RETR %s' % filename, open(filename, 'wb').write)
ftp.storbinary('STOR %s' % filename1, open(filename1, 'rb'))
# ftp.sendcmd('MKD testfolder')
# ftp.sendcmd('PWD')
# ftp.sendcmd('CWD testfolder')
# ftp.sendcmd('MKD testfolder')
# ftp.sendcmd('RMD testfolder')
ftp.dir()
ftp.quit()
#print(ftp.getwelcome())
