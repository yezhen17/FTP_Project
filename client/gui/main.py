##################################################################
# entry point of the project, also contains the FTPCLient class,
# which manages the FTP core functions and GUI logic
##################################################################

import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QTreeWidget, QTreeWidgetItem, QMessageBox, QTextBrowser
from PyQt5 import QtGui
from windows import *
from ftp_core import *


class FTPClient():
    def __init__(self):
        super(FTPClient, self).__init__()
        self.homeWin = None
        self.resultWin = None
        #self.funcWin = FuncWindow()
        #self.client = Client()
        self.new_home()
        self.funcWin.pasvBtn.clicked.connect(self.set_pasv)
        self.funcWin.portBtn.clicked.connect(self.set_port)
        self.funcWin.actionSet_binary_mode.triggered.connect(self.set_type)
        self.funcWin.actionSystem_Info.triggered.connect(self.ask_syst)
        self.funcWin.downloadBtn.clicked.connect(self.download_file)
        self.funcWin.close_signal.connect(self.close_connect)
        self.funcWin.mkdirBtn.clicked.connect(self.mkdir)
        self.funcWin.removeBtn.clicked.connect(self.rmdir)
        self.funcWin.uploadButton.clicked.connect(self.upload_file)
        self.funcWin.renameBtn.clicked.connect(self.rename_file)
        self.funcWin.pauseButton.clicked.connect(self.close_transfer)
        self.client.respBox = self.funcWin.serverResp
        self.client.win = self.funcWin
        self.last_src = None
        self.last_dest = None
        self.last_size = None
        self.last_offset = None
        self.isTransfering = False

    def new_home(self):
        self.homeWin = HomeWindow()
        self.client = Client()
        self.funcWin = FuncWindow()
        self.homeWin.show()
        self.homeWin.lineIP.setText('166.111.82.233') # convenient for testing
        self.homeWin.linePort.setText('9999')
        self.homeWin.lineEdit.returnPressed.connect(self.login)
        self.homeWin.pushButton.clicked.connect(self.login)

    '''
    connects FTP server and logs in successively
    '''
    def login(self):
        login_res = self.client.new_connect(self.homeWin.lineIP.text(), int(self.homeWin.linePort.text()))
        if login_res[0]:
            print("yessir")
            if self.client.login(self.homeWin.lineEdit.text()):
                self.homeWin.close()
                self.funcWin.show()
                self.init_filelist(None)
            else:
                QMessageBox.information(None, 'Error', 'Error logging in.', QMessageBox.Yes)
        else:
            QMessageBox.information(None, 'Error', login_res[1], QMessageBox.Yes)

    '''
    set PASV mode
    '''
    def set_pasv(self):
        self.client.pasv_mode()
        self.funcWin.portBtn.setChecked(False)

    '''
    set PORT mode
    '''
    def set_port(self):
        self.client.port_mode()
        self.funcWin.pasvBtn.setChecked(False)

    '''
    sets TYPE through menubar, though it is useless since default is already binary mode
    '''
    def set_type(self):
        if self.isTransfering:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        self.funcWin.actionSet_binary_mode.setText('Set binary mode √')
        self.client.type_cmd()

    '''
    checks SYST info
    '''
    def ask_syst(self):
        if self.isTransfering:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        self.client.syst_cmd()

    '''
    closes connection with client, closes current window, and shows a result window
    '''
    def close_connect(self):
        self.client.quit_cmd()
        print(self.client.resp)
        self.resultWin = ResultWindow()
        self.resultWin.show()
        self.resultWin.textBrowser.setText(self.client.resp)

    '''
    downloads file without typing directory
    '''
    def download_file(self):
        if self.isTransfering:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        if self.funcWin.pauseButton.text() == 'Resume':
            self.funcWin.pauseButton.setText('Pause')
            self.funcWin.pauseButton.clicked.disconnect(self.resume_transfer)
            self.funcWin.pauseButton.clicked.connect(self.close_transfer)
        selected_dir = self.funcWin.treeWidget.currentItem()
        if selected_dir is None or selected_dir.text(0) == '..' or selected_dir.text(1) != 'File':
            QMessageBox.information(None, 'Error', 'Not a file.', QMessageBox.Yes)
            return
        filesize = int(selected_dir.text(3))
        src_path = selected_dir.text(0).strip()
        if self.client.prefix[-1] == '/':
            src_path = self.client.prefix + src_path
        else:
            src_path = self.client.prefix + '/' + src_path
        dest_path = QFileDialog.getExistingDirectory(None, "Choose a folder.", os.getcwd())
        print(src_path)

        # memorizes the last download/upload operation to enable transmission resume
        self.last_dest = dest_path
        self.last_src = src_path
        self.last_size = filesize
        self.last_offset = None

        self.start_download(src_path, dest_path, filesize)

    '''
    starts downloading
    '''
    def start_download(self, src_path, dest_path, filesize):
        self.isTransfering = True

        self.client.download_file(src_path, dest_path, self.funcWin.downloadBar, filesize)

        def download_complete():
            self.isTransfering = False

        self.client.download_thread.complete_signal.connect(download_complete)

    '''
    uploads file without typing directory
    '''
    def upload_file(self):
        if self.isTransfering:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        if self.funcWin.pauseButton.text() == 'Resume':
            self.funcWin.pauseButton.setText('Pause')
            self.funcWin.pauseButton.clicked.disconnect(self.resume_transfer)
            self.funcWin.pauseButton.clicked.connect(self.close_transfer)
        selected_dir = self.funcWin.treeWidget.currentItem()
        dest_folder = ''
        if selected_dir is not None:
            dest_folder = selected_dir.text(0)
            if dest_folder == '..' or selected_dir.text(1) != 'Folder':
                QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
                return

        chosen_file = QFileDialog.getOpenFileName(None, "Choose file to upload.", os.getcwd())
        if not chosen_file[0]:
            return
        src_path = chosen_file[0]

        # if no folder chosen, use the parent folder
        if dest_folder:
            dest_path = dest_folder + '/' + os.path.basename(src_path)
        else:
            dest_path = os.path.basename(src_path)

        # convert to absolute path
        if self.client.prefix[-1] == '/':
            dest_path = self.client.prefix + dest_path
        else:
            dest_path = self.client.prefix + '/' + dest_path

        # memorizes the last download/upload operation to enable transmission resume
        self.last_dest = dest_path
        self.last_src = src_path
        self.last_size = None

        self.start_upload(src_path, dest_path, 0) # zero offset

    '''
    starts uploading
    '''
    def start_upload(self, src_path, dest_path, offset):
        self.isTransfering = True
        self.client.upload_file(src_path, dest_path, self.funcWin.downloadBar, offset)

        def upload_complete():
            self.isTransfering = False
            self.funcWin.treeWidget.clear()
            self.unfold()

        self.client.upload_thread.complete_signal.connect(upload_complete)

    '''
    use list command to show directory info from the server
    '''
    def unfold(self):
        root = self.funcWin.treeWidget

        # the root folder doesn't have .. for security issues
        if self.client.prefix != self.client.root:
            ret = QTreeWidgetItem(root)
            ret.setText(0, '..')
            ret.isFile = '..'

        flst = self.client.list_cmd(None)
        print(flst)
        if flst == '':
            return
        flst = flst.split('\n')[:-1]
        items = []
        for file in flst:
            parsed_info = []
            file = file.split(' ')
            for i in range(len(file)):
                if file[i] != '':
                    parsed_info.append(file[i])
            print(parsed_info)
            item = QTreeWidgetItem(root)
            item.setText(0, parsed_info[8].strip('\r'))
            item.setText(2, ' '.join(parsed_info[5:8]))

            if parsed_info[0][0] == '-':
                item.setText(3, parsed_info[4])
                item.setText(1, 'File')
                item.isFile = 'yes'
            elif parsed_info[0][0] == 'd':
                item.setText(1, 'Folder')
                item.isFile = 'no'
            items.append(item)
        return items

    '''
    initializes directory list
    '''
    def init_filelist(self, dir):
        def refresh():
            if self.isTransfering:
                QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
                return
            root = self.funcWin.treeWidget
            ftp = self.client
            file = root.currentItem()
            path = file.text(0)
            print(file.isFile)

            # change a parent directory
            if file.isFile == '..':
                root.clear()
                ftp.cwd_cmd(self.funcWin.treeWidget.parent)
                ftp.pwd_cmd()
                self.unfold()

            if file.isFile == 'no':
                root.clear()
                root.parent = ftp.prefix
                ftp.cwd_cmd(ftp.prefix + '/' + path)
                ftp.pwd_cmd()
                self.unfold()

        root = self.funcWin.treeWidget
        root.setHeaderLabels(['Filename', 'Type', 'Last modified', 'Size(Bytes)'])
        root.doubleClicked.connect(refresh) # doubleclick on folder to enter
        root.expandAll()
        root.parent = None
        self.client.pwd_cmd() # use PWD command to get prefix
        self.client.root = self.client.prefix
        self.unfold()

    '''
    creates a new folder
    '''
    def mkdir(self):
        if self.isTransfering:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        self.client.mkd_cmd(self.funcWin.newdirLine.text())
        self.funcWin.treeWidget.clear()
        self.unfold()

    '''
    removes a folder recursively or removes a file
    '''
    def rmdir(self):
        if self.isTransfering:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        ftp = self.client
        root = self.funcWin.treeWidget
        selected_file = root.selectedItems()[0]
        path = selected_file.text(0)
        print(len(path))
        ftp.rmd_cmd(path)
        root.clear()
        self.unfold()

    '''
    removes a folder recursively or removes a file
    '''
    def rename_file(self):
        if self.isTransfering:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        ftp = self.client
        root = self.funcWin.treeWidget
        selected_file = root.selectedItems()[0]
        path = selected_file.text(0)
        ftp.rnfr_cmd(path)
        if ftp.code != 350:
            return
        newpath = self.funcWin.renameLine.text()
        ftp.rnto_cmd(newpath)
        root.clear()
        self.unfold()

    '''
    pauses a ongoing file transfer
    '''
    def close_transfer(self):
        if not self.isTransfering:
            QMessageBox.information(None, 'Error', 'No file transfering!', QMessageBox.Yes)
            return
        # self.abor_signal = pyqtSignal()
        # print("1")
        # self.abor_signal.connect(self.client.uploadthd.abor)
        # print("2")
        # self.abor_signal.emit()
        self.funcWin.pauseButton.setText("Resume")
        self.funcWin.pauseButton.clicked.disconnect(self.close_transfer)
        self.funcWin.pauseButton.clicked.connect(self.resume_transfer)
        # upload
        if self.last_size is None:
            self.client.abor_cmd()
            time.sleep(0.5)
        #print("sum",self.client.uploadthd.sum)
            self.funcWin.treeWidget.clear()
            dirs = self.unfold()
            try:
                for dir in dirs:
                    if dir.text(0) == os.path.basename(self.last_src):
                        self.last_offset = int(dir.text(3))
            except Exception as e:
                print("2", str(e))
        else:
            print("downloading")
            self.client.send_cmd('ABOR')
            # self.client.recv_resp()
        print(self.last_offset)
        # self.client.data_s.close()
        # self.client.recv_resp()
        # self.last_offset = self.client.uploadthd.sum
        # self.funcWin.treeWidget.clear()
        # print("what", self.last_offset)
        # self.unfold()

    '''
    switches from pause to resume
    '''
    def resume_transfer(self):
        self.funcWin.pauseButton.setText("Pause")
        if self.last_offset is None:
            self.start_download(self.last_src, self.last_dest, self.last_size)
            #self.client.download_file(self.last_src, self.last_dest, self.funcWin.downloadBar, self.last_size)
        else:
            self.start_upload(self.last_src, self.last_dest, self.last_offset)
            #self.client.upload_file(self.last_src, self.last_dest, self.funcWin.downloadBar, self.last_offset)
        print("resume from", self.last_offset)
        self.funcWin.pauseButton.clicked.disconnect(self.resume_transfer)
        self.funcWin.pauseButton.clicked.connect(self.close_transfer)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    ftpClient = FTPClient()
    sys.exit(app.exec_())
