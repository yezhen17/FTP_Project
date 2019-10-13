from PyQt5.QtWidgets import QApplication, QMainWindow, QFileDialog
from func import *
from result import *
from test import *
import os


class FuncWindow(QMainWindow, Ui_func):
    def __init__(self, parent=None):
        super(FuncWindow, self).__init__(parent)
        self.setupUi(self)
        self.pasvBtn.setChecked(True)
        self.dir_btn.clicked.connect(self.chooseDir)
        self.cwd = os.getcwd()
        self.local_dir = None
    close_signal = QtCore.pyqtSignal(str)

    def closeEvent(self, event):
        self.close_signal.emit('close')

    def chooseDir(self):
        dir_choose = QFileDialog.getExistingDirectory(self,
                                                      "选取文件夹",
                                                      self.cwd)  # 起始路径

        if dir_choose == "":
            return

        self.local_dir = dir_choose


class ResultWindow(QMainWindow, Ui_Result):
    def __init__(self, parent=None):
        super(ResultWindow, self).__init__(parent)
        self.setupUi(self)

    close_signal = QtCore.pyqtSignal(str)

    def closeEvent(self, event):
        self.close_signal.emit('close')


class HomeWindow(QMainWindow, Ui_home):
    def __init__(self, parent=None):
        super(HomeWindow, self).__init__(parent)
        self.setupUi(self)
        self.lineEdit.returnPressed.connect(self.login)
        #self.myButton.clicked.connect(self.myPrint)
    def login(self):
        self.close()