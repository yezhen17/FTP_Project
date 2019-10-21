import ftplib as ftp
import socket
import os
import random
import struct
import time
from transfer import downloadThread, uploadThread


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
        self.uploadthd = None
        self.downloadthd = None

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

    def quit_cmd(self):
        self.send_cmd('QUIT')
        self.recv_resp()
        self.s.close()

    def abor_cmd(self):
        #self.send_cmd('ABOR')
        self.data_s.close()



    @after_func
    def download_file(self, src_path, dest_path, bar, size):
        if self.isPasv:
            self.pasv_cmd()
        else:
            self.port_cmd()

        self.retr_cmd(src_path, dest_path, bar, size)


    @after_func
    def pasv_cmd(self):
        self.send_cmd('PASV')
        self.recv_resp()
        addr = self.resp.split('(')[-1].split(')')[0]
        # print(self.resp)
        addr = addr.split(',')
        self.data_addr = ('.'.join(addr[:4]), int(addr[4]) * 256 + int(addr[5]))
        print(self.data_addr)

    @after_func
    def port_cmd(self):
        port = get_free_port(self.this_ip)
        cmd = 'PORT ' + self.this_ip.replace('.', ',') + ',' + str(port // 256) + ',' + str(port % 256)
        print(cmd)
        self.send_cmd(cmd)
        self.recv_resp()
        print("sb?")
        self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.data_s.bind((self.this_ip, port))
        self.data_s.listen(10)
        print("fuck")

        print("hello",self.resp)


    def upload_file(self, src_path, dest_path, bar, size):
        resp = ''
        if self.isPasv:
            self.pasv_cmd()
        else:
            self.port_cmd()
        resp += self.resp + '\r\n'
        self.stor_cmd(src_path, dest_path, bar, size)
        resp += self.resp

    @after_func
    def retr_cmd(self, src_path, dest_path, bar, size):
        print(src_path)
        dir = os.path.join(dest_path, os.path.basename(src_path))
        if os.path.exists(dir):
            cursize = os.path.getsize(dir)
        else:
            cursize = 0
        print(cursize)
        self.send_cmd('REST ' + str(cursize))
        self.recv_resp()
        print(cursize)
        self.send_cmd('RETR ' + src_path)

        #print(code)
        if self.isPasv:
            self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                self.data_s.connect(self.data_addr)
            except:
                print("something wrong")
                return
        else:
            new_s, _ = self.data_s.accept()
            self.data_s.close()
            self.data_s = new_s
        self.recv_resp()
        def update_bar(sum):
            bar.setValue(sum * 100 / size)
        self.downloadthd = downloadThread(self, size, cursize, dir)
        self.downloadthd.bar_signal.connect(update_bar)
        #self.downloadThread.download_proess_signal.connect(self.set_progressbar_value)
        self.downloadthd.start()
        # sum = cursize
        # try:
        #     #if os.path.exists(dir):
        #     #    os.remove(dir)
        #     with open(dir, 'ab+') as f:
        #         print(dir)
        #         while True:
        #             data = self.data_s.recv(8092)
        #             # print(data)
        #             if not data:
        #                 break
        #             sum += len(data)
        #             bar.setValue(sum * 100 / size)
        #             f.write(data)
        # except Exception as e:
        #     print(str(e))


    @after_func
    def stor_cmd(self, src_path, dest_path, bar, cursize):

        self.send_cmd('REST ' + str(cursize))
        self.recv_resp()
        print(self.resp.split('\n')[-2])
        size = os.path.getsize(src_path)

        self.send_cmd('STOR ' + dest_path)

        if self.isPasv:
            self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.data_s.connect(self.data_addr)
        else:
            new_s, _ = self.data_s.accept()
            self.data_s.close()
            self.data_s = new_s
        print(dest_path)
        self.recv_resp()
        print(self.resp.split('\n')[-2])
        def update_bar(sum):
            bar.setValue(sum * 100 / size)
        self.uploadthd = uploadThread(self, size, cursize, src_path)
        self.uploadthd.bar_signal.connect(update_bar)
        self.uploadthd.start()
        # with open(src_path, 'rb') as f:
        #     sum = 0
        #     while True:
        #         data = f.read(8092)
        #         #print(data)
        #         if not data:
        #             break
        #         self.data_s.send(data)
        #         sum += len(data)
        #         bar.setValue(sum * 100 / size)
        # self.data_s.close()
        # print("yes")
        # self.recv_resp()

    @after_func
    def mkd_cmd(self, dest_path):
        self.send_cmd('MKD ' + dest_path)
        self.recv_resp()

    @after_func
    def rmd_cmd(self, dest_path):
        self.send_cmd('RMD ' + dest_path)
        self.recv_resp()

    @after_func
    def rnfr_cmd(self, dest_path):
        self.send_cmd('RNFR ' + dest_path)
        self.recv_resp()

    @after_func
    def rnto_cmd(self, dest_path):
        self.send_cmd('RNTO ' + dest_path)
        self.recv_resp()

    @after_func
    def pwd_cmd(self):
        self.send_cmd('PWD')
        self.recv_resp()
        self.prefix = self.resp.split('\n')[-2].split(' ')[1].strip()[1:-1]
        print("!!!!",self.prefix)


    @after_func
    def cwd_cmd(self, dest_path):
        self.send_cmd('CWD ' + dest_path)
        self.recv_resp()

    @after_func
    def list_cmd(self, dest_path):
        print("not sent yet")
        if self.isPasv:
            self.pasv_cmd()
        else:
            self.port_cmd()
        #print(self.resp.split('\n')[-2])
        #print("sent pasv")
        if dest_path is not None:
            self.send_cmd('LIST ' + dest_path)
        else:
            self.send_cmd('LIST')
        if self.isPasv:
            self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                self.data_s.connect(self.data_addr)
            except Exception as e:
                print("hey", e)
                return ''
        else:
            try:
                print(self.data_s)
                new_s, _ = self.data_s.accept()
                print(new_s)
                self.data_s.close()
                self.data_s = new_s
            except Exception as e:
                print(str(e))
        print(dest_path)

        self.recv_resp()
        allfile = ''
        while True:
            data = self.data_s.recv(8192).decode()
            allfile += data
            #print(data)
            if not data:
                break
        self.recv_resp()
        self.data_s.close()
        return allfile
