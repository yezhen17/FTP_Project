###################################################
# three windows, inherit from the QtDesigner generated files
###################################################


from PyQt5.QtWidgets import QMainWindow
from func import *
from result import *
from test import *
import os


class FuncWindow(QMainWindow, Ui_func):
    close_signal = QtCore.pyqtSignal(str)

    def __init__(self, parent=None):
        super(FuncWindow, self).__init__(parent)
        self.setupUi(self)
        self.pasvBtn.setChecked(True)
        self.cwd = os.getcwd()
        self.local_dir = None
        self.renameBtn.setToolTip('Download a file from the server.')
        self.doClose = True
        self.setFixedSize(self.width(), self.height()) # fix the window size

    def closeEvent(self, event):

        if self.doClose:
            self.close_signal.emit('close')
            event.accept()
        else:
            self.close_signal.emit('ask')
            event.ignore()

    def finally_close(self):
        self.doClose = True
        self.close()


class ResultWindow(QMainWindow, Ui_Result):
    close_signal = QtCore.pyqtSignal(str)

    def __init__(self, parent=None):
        super(ResultWindow, self).__init__(parent)
        self.setupUi(self)

        def closeself():
            self.close()
        self.pushButton.clicked.connect(closeself)


class HomeWindow(QMainWindow, Ui_home):
    def __init__(self, parent=None):
        super(HomeWindow, self).__init__(parent)
        self.setupUi(self)
        self.lineEdit.setFocus()
        self.setFixedSize(self.width(), self.height())

    def login(self):
        self.close()