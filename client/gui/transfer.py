import sys
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import time
import socket
import queue     #如果不加载这个模板，pyinstaller打包后，可能无法运行requests模板
import requests
from ftp_core import *




class downloadThread(QThread):                       #创建信号
    bar_signal = pyqtSignal(int)
    def __init__(self, ftp, filesize, startsize, dir):
        super(downloadThread, self).__init__()
        self.ftp = ftp
        self.filesize = filesize
        self.startsize = startsize
        self.dir = dir
        print("here",filesize)

    def run(self):
        sum = self.startsize
        try:
            # if os.path.exists(dir):
            #    os.remove(dir)
            i = 0
            with open(self.dir, 'ab') as f:
                print("dir",self.dir)
                while True:
                    data = self.ftp.data_s.recv(8192)
                    #print(data)

                    if not data:
                        break
                    sum += len(data)
                    print(sum)
                    #if i % 20 == 0:
                    time.sleep(0.001)
                    self.bar_signal.emit(sum)
                    #    self.bar.setValue(sum * 100 / self.filesize)
                    f.write(data)
                #self.bar.setValue(sum * 100 / self.filesize)


        except Exception as e:
            print(str(e))
            #self.exit()
        #self.ftp.data_s.close()
        print("hello")
        self.ftp.recv_resp()
        print(self.ftp.resp.split('\n')[-2])
        self.exit()
        #print("thd", resp, "test")

class uploadThread(QThread):
    bar_signal = pyqtSignal(int)
    def __init__(self, ftp, filesize, startsize, dir):
        super(uploadThread, self).__init__()
        self.ftp = ftp
        #self.bar = bar
        self.filesize = filesize
        self.startsize = startsize
        print(self.startsize)
        self.dir = dir
        print(self.filesize)
        self.aborted = False
        self.sum = 0

    def abor(self):
        self.aborted = True


    def run(self):
        try:
            self.sum = self.startsize
            with open(self.dir, 'rb') as f:
                f.seek(self.startsize)
                i = 0
                while True:
                    data = f.read(8192)
                    # print(data)
                    if self.aborted:
                        break
                    if not data:
                        print("end")
                        break
                    if self.ftp.data_s.send(data) <=0:
                        print("abort")
                        break

                    #print(data)
                    self.sum += len(data)
                    time.sleep(0.001)
                    self.bar_signal.emit(self.sum)
                    #if i % 20 == 0:
                    #    self.bar.setValue(self.sum * 100 / self.filesize)
                    #stime.sleep(0.001)
                #self.bar.setValue(self.sum * 100 / self.filesize)
            self.ftp.data_s.close()
            print("hello")
            self.ftp.recv_resp()
            print(self.ftp.resp.split('\n')[-2])

            #self.complete_signal.emit(self.sum)

        except Exception as e:
            print(str(e))
            print(self.sum)
            self.ftp.recv_resp()
            print(self.ftp.resp.split('\n')[-2])
            #self.complete_signal.emit(self.sum)
        #self.ftp.data_s.close()
        self.exit()
        #print("hello")
        #self.ftp.recv_resp()
        #self.complete_signal.emit(sum)
        #print("thd", resp, "test")







