import ftplib as ftp
import socket
import os
import random

def get_free_port(ip):
    while True:
        port = random.randint(20000, 65535)
        test_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            test_s.bind((ip, port))
        except:
            continue
        return port

def get_local_ip():
    """
    查询本机ip地址
    :return:
    """
    try:
        s = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
    finally:
        s.close()
    return ip

class Client():
    def __init__(self):
        super(Client, self).__init__()
        self.this_ip = get_local_ip()
        self.host = ('166.111.82.233', 6789)
        self.isPasv = True
        self.resp = ''
        self.s = None
        self.data_addr = None
        self.data_s = None

    def new_connect(self):
        try:
            self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except:
            print("Error create socket")
        try:
            self.s.connect(self.host)
            self.s.recv(1024)
        except:
            self.s.close()
            print("Error connect")
        self.isPasv = True
        self.resp = ''

    def send_cmd(self, cmd):
        cmd += '\r\n'
        self.s.send(cmd.encode('utf-8'))

    def recv_resp(self):
        resp = self.s.recv(1024).decode()
        last_line = resp.split('\r\n')[-2]
        code = int(last_line.split(' ')[0])
        return code, resp[:-2]

    def login(self, passwd):
        self.send_cmd('USER anonymous')
        code, resp1 = self.recv_resp()
        if code != 331:
            return 0
        self.send_cmd('PASS ' + passwd)
        code, resp2 = self.recv_resp()
        if code != 230:
            return 0
        self.resp = resp1 + '\r\n' + resp2
        return 1

    def port_mode(self):
        self.isPasv = False

    def pasv_mode(self):
        self.isPasv = True

    def type_cmd(self):
        self.send_cmd('TYPE I')
        _, self.resp = self.recv_resp()

    def syst_cmd(self):
        self.send_cmd('SYST')
        _, self.resp = self.recv_resp()

    def quit_cmd(self):
        self.send_cmd('QUIT')
        _, self.resp = self.recv_resp()
        self.s.close()

    def download_file(self, src_path, dest_path):
        resp = ''
        if self.isPasv:
            self.pasv_cmd()
        else:
            self.port_cmd()
        resp += self.resp
        self.retr_cmd(src_path, dest_path)
        resp += self.resp
        self.resp = resp

    def pasv_cmd(self):
        self.send_cmd('PASV')
        _, self.resp = self.recv_resp()
        addr = self.resp.split(' ')[-1][:-2]
        addr = addr.split(',')
        self.data_addr = ('.'.join(addr[:3]), int(addr[4] * 256 + addr[5]))

    def port_cmd(self):
        port = get_free_port(self.this_ip)
        self.send_cmd('PORT ' + self.this_ip.replace('.', ',') + ',' + str(port / 256) + ',' + str(port % 256))
        _, self.resp = self.recv_resp()
        self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.data_s.bind((self.this_ip, port))
        self.data_s.listen(1)

    def upload_file(self, src_path, dest_path):
        if self.isPasv:
            self.pasv_cmd()
        else:
            self.port_cmd()
        self.stor_cmd(src_path, dest_path)

    def retr_cmd(self, src_path, dest_path):
        if self.isPasv:
            self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.data_s.connect(self.data_addr)
        self.send_cmd('RETR ' + src_path)
        with open(dest_path, 'wb') as f:
            while True:
                data = self.data_s.recv(8092)
                if not data:
                    break
                f.write(data)
        _, self.resp = self.recv_resp()

    def stor_cmd(self, src_path, dest_path):
        if self.isPasv:
            self.data_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.data_s.connect(self.data_addr)
        self.send_cmd('STOR ' + src_path)
        with open(dest_path, 'rb') as f:
            while True:
                data = f.read(8092)
                if not data:
                    break
                self.data_s.send(data)
        _, self.resp = self.recv_resp()

    def mkd_cmd(self, dest_path):
        self.send_cmd('MKD ' + dest_path)
        _, self.resp = self.recv_resp()

    def pwd_cmd(self):
        self.send_cmd('PWD')
        _, self.resp = self.recv_resp()

    def cwd_cmd(self, dest_path):
        self.send_cmd('CWD '+dest_path)


