import ftplib as ftp
import socket
import os
import random
import struct
import time
from PyQt5.QtWidgets import QMessageBox
from transfer import DownloadThread, UploadThread


'''
decorator function, used to append server response to text browser
'''
def after_func(func):
    def wrapper(self, *args):
        ret = func(self, *args)
        self.respBox.setText(self.resp)
        self.respBox.moveCursor(self.respBox.textCursor().End)
        return ret
    return wrapper


'''
used to choose a random free port between 20000 and 65535
'''
def get_free_port(ip):
    while True:
        port = random.randint(20000, 65535)
        test_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            test_s.bind((ip, port))
        except:
            continue
        return port


'''
get the ip of this machine (not 127.0.0.1)
'''
def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
    finally:
        s.close()
    return ip


class Client():
    def __init__(self):
        super(Client, self).__init__()
        self.this_ip = get_local_ip()
        self.host = None
        #self.host = ('166.111.82.233', 9999)
        self.isPasv = True
        self.resp = ''
        self.code = None
        self.s = None
        self.data_addr = None
        self.data_s = None
        self.prefix = None
        self.root = None
        self.respBox = None
        self.upload_thread = None
        self.download_thread = None

    '''
    processes command
    '''
    def send_cmd(self, cmd):
        cmd += '\r\n'
        self.s.send(cmd.encode('utf-8'))

    '''
    processes response
    '''
    def recv_resp(self):
        resp = self.s.recv(1024).decode()
        last_line = resp.split('\r\n')[-2]
        self.code = int(last_line.split(' ')[0])
        self.resp += resp
        return last_line

    '''
    connects and ftp server
    '''
    @after_func
    def new_connect(self, ip, port):
        try:
            self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except Exception as e:
            return False, str(e)
        try:
            self.host = (ip, port)
            self.s.connect(self.host)
            self.recv_resp()
        except Exception as e:
            self.s.close()
            return False, str(e)
        return True, ''

    '''
    anonymous user login
    param: passwd(str)
    '''
    @after_func
    def login(self, passwd):
        self.send_cmd('USER anonymous')
        self.recv_resp()
        if self.code != 331:
            return False
        self.send_cmd('PASS ' + passwd)
        self.recv_resp()
        if self.code != 230:
            return False
        return True

    '''
    switches to port mode, doesn't send cmd
    '''
    def port_mode(self):
        self.isPasv = False

    '''
    switches to pasv mode, doesn't send cmd
    '''
    def pasv_mode(self):
        self.isPasv = True

    '''
    TYPE command
    '''
    @after_func
    def type_cmd(self):
        self.send_cmd('TYPE I')
        self.recv_resp()

    '''
    SYST command
    '''
    @after_func
    def syst_cmd(self):
        self.send_cmd('SYST')
        self.recv_resp()

    '''
    QUIT command
    '''
    @after_func
    def quit_cmd(self):
        self.send_cmd('QUIT')
        self.recv_resp()
        self.s.close()

    '''
    ABOR command
    '''
    def abor_cmd(self):
        #self.send_cmd('ABOR')
        self.data_s.close()

    '''
    download file, first send TYPE I, then send PORT or PASV, 
    then send REST (or not), finally send RETR
    param: src_path(str), dest_path(str), bar(ProgressBar), size(int)
    '''
    def download_file(self, src_path, dest_path, bar, filesize):
        self.type_cmd()
        if self.isPasv:
            self.pasv_cmd()
        else:
            self.port_cmd()

        print(src_path)
        dir = os.path.join(dest_path, os.path.basename(src_path))
        if os.path.exists(dir):
            offset = os.path.getsize(dir)
            self.rest_cmd(offset) # if offset is 0, don't send REST since it is useless
        else:
            offset = 0
        print(offset)

        self.retr_cmd(src_path, dir, bar, filesize, offset)

    '''
    upload file, first send TYPE I, then send PORT or PASV, 
    then send REST (or not), finally send STOR
    param: src_path(str), dest_path(str), bar(ProgressBar), size(int)
    '''
    def upload_file(self, src_path, dest_path, bar, filesize):
        self.type_cmd()
        if self.isPasv:
            self.pasv_cmd()
        else:
            self.port_cmd()
        if filesize != 0:
            self.rest_cmd(filesize)
        self.stor_cmd(src_path, dest_path, bar, filesize)

    '''
    REST command
    param: offset(int)
    '''
    @after_func
    def rest_cmd(self, offset):
        self.send_cmd('REST ' + str(offset))
        self.recv_resp()

    '''
    PASV command
    '''
    @after_func
    def pasv_cmd(self):
        self.send_cmd('PASV')
        self.recv_resp()

        # parse the response
        addr = self.resp.split('(')[-1].split(')')[0]
        addr = addr.split(',')
        self.data_addr = ('.'.join(addr[:4]), int(addr[4]) * 256 + int(addr[5]))

    '''
    PORT command
    '''
    @after_func
    def port_cmd(self):
        # form the command
        port = get_free_port(self.this_ip)
        cmd = 'PORT ' + self.this_ip.replace('.', ',') + ',' + str(port // 256) + ',' + str(port % 256)
        self.send_cmd(cmd)
        self.recv_resp()

        # start listening
        self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.data_s.bind((self.this_ip, port))
        self.data_s.listen(5)

    '''
    RETR command
    param: src_path(str), dest_path(str), bar(ProgressBar), filesize(int), offset(int)
    '''
    @after_func
    def retr_cmd(self, src_path, dest_path, bar, filesize, offset):
        # print(src_path)
        # dir = os.path.join(dest_path, os.path.basename(src_path))
        # if os.path.exists(dir):
        #     cursize = os.path.getsize(dir)
        # else:
        #     cursize = 0
        # print(cursize)

        self.send_cmd('RETR ' + src_path)

        # establish data transfer connection
        if self.isPasv:
            self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                self.data_s.connect(self.data_addr)
            except Exception as e:
                QMessageBox.information(None, 'Error', str(e), QMessageBox.Yes)
                return
        else:
            try:
                new_s, _ = self.data_s.accept()
                self.data_s.close()
                self.data_s = new_s
            except Exception as e:
                QMessageBox.information(None, 'Error', str(e), QMessageBox.Yes)
                return

        self.recv_resp()

        def update_bar(progress):
            bar.setValue(progress * 100 / filesize)

        self.download_thread = DownloadThread(self, filesize, offset, dest_path)
        self.download_thread.bar_signal.connect(update_bar)
        self.download_thread.start()


    '''
    STOR command
    param: src_path(str), dest_path(str), bar(ProgressBar), filesize(int), offset(int)
    '''
    @after_func
    def stor_cmd(self, src_path, dest_path, bar, offset):
        filesize = os.path.getsize(src_path)
        self.send_cmd('STOR ' + dest_path)

        # establish data transfer connection
        if self.isPasv:
            self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                self.data_s.connect(self.data_addr)
            except Exception as e:
                QMessageBox.information(None, 'Error', str(e), QMessageBox.Yes)
                return
        else:
            try:
                new_s, _ = self.data_s.accept()
                self.data_s.close()
                self.data_s = new_s
            except Exception as e:
                QMessageBox.information(None, 'Error', str(e), QMessageBox.Yes)
                return
        self.recv_resp()

        def update_bar(progress):
            bar.setValue(progress * 100 / filesize)

        self.upload_thread = UploadThread(self, filesize, offset, src_path)
        self.upload_thread.bar_signal.connect(update_bar)
        self.upload_thread.start()

    '''
    MKD command
    '''
    @after_func
    def mkd_cmd(self, dest_path):
        self.send_cmd('MKD ' + dest_path)
        self.recv_resp()

    '''
    RMD command
    '''
    @after_func
    def rmd_cmd(self, dest_path):
        self.send_cmd('RMD ' + dest_path)
        self.recv_resp()

    '''
    RNFR command
    '''
    @after_func
    def rnfr_cmd(self, dest_path):
        self.send_cmd('RNFR ' + dest_path)
        self.recv_resp()

    '''
    RNTO command
    '''
    @after_func
    def rnto_cmd(self, dest_path):
        self.send_cmd('RNTO ' + dest_path)
        self.recv_resp()

    '''
    PWD command
    '''
    @after_func
    def pwd_cmd(self):
        self.send_cmd('PWD')
        line = self.recv_resp()
        self.prefix = line.split(' ')[1].strip()[1:-1] # strip \"

    '''
    CWD command
    param: dest_path(str)
    '''
    @after_func
    def cwd_cmd(self, dest_path):
        self.send_cmd('CWD ' + dest_path)
        self.recv_resp()

    '''
    LIST command
    param: dest_path(str)
    '''
    @after_func
    def list_cmd(self, dest_path):
        # although a standard client uses TYPE A for LIST commands, we can use TYPE I
        if self.isPasv:
            self.pasv_cmd()
        else:
            self.port_cmd()

        if dest_path is not None:
            self.send_cmd('LIST ' + dest_path)
        else:
            self.send_cmd('LIST')

        if self.isPasv:
            self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                self.data_s.connect(self.data_addr)
            except Exception as e:
                QMessageBox.information(None, 'Error', str(e), QMessageBox.Yes)
                return ''
        else:
            try:
                new_s, _ = self.data_s.accept()
                self.data_s.close()
                self.data_s = new_s
            except Exception as e:
                QMessageBox.information(None, 'Error', str(e), QMessageBox.Yes)
                return ''
        self.recv_resp()

        list = ''
        while True:
            data = self.data_s.recv(8192).decode()
            list += data
            if not data:
                break
        self.recv_resp()
        self.data_s.close()
        return list
