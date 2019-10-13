import sys
from PyQt5.QtWidgets import QApplication, QMainWindow

from windows import *

from ftp_core import *


def after_func(func):
    def wrapper(self):
        func(self)
        self.show_resp()
    return wrapper




class FTPClient():
    def __init__(self):
        super(FTPClient, self).__init__()
        self.homeWin = None
        self.resultWin = None
        self.funcWin = FuncWindow()
        self.client = Client()
        #self.client.new_connect()
        self.new_home()
        self.funcWin.pasvBtn.clicked.connect(self.set_pasv)
        self.funcWin.portBtn.clicked.connect(self.set_port)
        self.funcWin.actionSet_binary_mode.triggered.connect(self.set_type)
        self.funcWin.actionSystem_Info.triggered.connect(self.ask_syst)
        #self.funcWin.close.connect(self.printf)
        self.funcWin.close_signal.connect(self.close_connect)

    def new_home(self):
        self.homeWin = HomeWindow()
        self.homeWin.show()
        self.homeWin.lineEdit.returnPressed.connect(self.login)
    # decorator function, used to print server response on text browser
    def show_resp(self):
        #print(self.client.resp)

        self.funcWin.serverResp.append(self.client.resp)

    @after_func
    def login(self):
        self.client.new_connect()
        if self.client.login(self.homeWin.lineEdit.text()):
            self.funcWin.show()

    def set_pasv(self):
        self.client.pasv_mode()
        self.funcWin.portBtn.setChecked(False)

    def set_port(self):
        self.client.port_mode()
        self.funcWin.pasvBtn.setChecked(False)

    @after_func
    def set_type(self):
        self.funcWin.actionSet_binary_mode.setText('Set binary mode √')
        self.client.type_cmd()

    @after_func
    def ask_syst(self):
        self.client.syst_cmd()

    def close_connect(self):
        self.client.quit_cmd()
        print(self.client.resp)
        self.resultWin = ResultWindow()
        self.resultWin.show()
        self.resultWin.textBrowser.setText(self.client.resp)
        self.resultWin.close_signal.connect(self.new_home)

    def download_file(self):

        src_path = self.funcWin.local_dir
        dest_path = 'out.txt'
        self.client.download_file(src_path, dest_path)



if __name__ == '__main__':
    app = QApplication(sys.argv)
    # homeWin = HomeWindow()
    # funcWin = FuncWindow()
    # client = Client()
    # homeWin.lineEdit.returnPressed.connect(funcWin.show)
    # homeWin.lineEdit.returnPressed.connect(client.login)
    # homeWin.show()
    ftpClient = FTPClient()
    sys.exit(app.exec_())
