################################################
#serves for multi-thread downloading/uploading
################################################

from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import socket
from ftp_core import *

class DownloadThread(QThread):
    bar_signal = pyqtSignal(int)
    complete_signal = pyqtSignal()
    def __init__(self, ftp, filesize, offset, dir):
        super(DownloadThread, self).__init__()
        self.ftp = ftp
        self.filesize = filesize
        self.offset = offset
        self.dir = dir
        print("here",filesize)

    def run(self):
        progress = self.offset
        try:
            with open(self.dir, 'ab') as f:
                print("dir",self.dir)
                while True:
                    data = self.ftp.data_s.recv(8192)
                    if not data:
                        break
                    progress += len(data)
                    time.sleep(0.001) # control the speed to avoid jamming
                    self.bar_signal.emit(progress)
                    f.write(data)
        except Exception as e:
            print(str(e))
            #self.exit()
        #self.ftp.data_s.close()
        print("hello")
        self.ftp.recv_resp()
        print(self.ftp.resp.split('\n')[-2])
        self.complete_signal.emit()
        self.exit()


class UploadThread(QThread):
    bar_signal = pyqtSignal(int)
    complete_signal = pyqtSignal()
    def __init__(self, ftp, filesize, offset, dest_path):
        super(UploadThread, self).__init__()
        self.ftp = ftp
        self.filesize = filesize
        self.offset = offset
        self.dest_path = dest_path

    def run(self):
        try:
            progress = self.offset
            with open(self.dest_path, 'rb') as f:
                f.seek(self.offset)
                while True:
                    data = f.read(8192)
                    if not data:
                        break
                    if self.ftp.data_s.send(data) <= 0:
                        break
                    progress += len(data)
                    time.sleep(0.001)
                    self.bar_signal.emit(progress)
            self.ftp.data_s.close()
            self.ftp.recv_resp()
        except Exception as e:
            print(str(e))
            self.ftp.recv_resp()

        self.complete_signal.emit()
        self.exit()
