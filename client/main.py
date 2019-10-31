##################################################################
# entry point of the project, also contains the FTPCLient class,
# which manages the FTP core functions and GUI logic
##################################################################

import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QTreeWidget, QTreeWidgetItem, QMessageBox, QTextBrowser, QFileDialog
from PyQt5 import QtGui
from windows import *
from ftp_core import *


class FTPClient():
    def __init__(self):
        super(FTPClient, self).__init__()
        self.homeWin = None
        self.resultWin = None
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
        self.funcWin.downloadPause.clicked.connect(self.close_download_transfer)
        self.funcWin.uploadPause.clicked.connect(self.close_upload_transfer)
        self.client.respBox = self.funcWin.serverResp
        self.client.win = self.funcWin
        self.last_src = None
        self.last_dest = None
        self.last_size = None
        self.last_offset = None
        self.uploading = False
        self.downloading = False
        self.closing = False
        socket.setdefaulttimeout(5)

    '''
    initializes homewindow
    '''
    def new_home(self):
        self.homeWin = HomeWindow()
        self.client = Client()
        self.funcWin = FuncWindow()
        self.homeWin.show()
        self.homeWin.lineIP.setText('166.111.82.233') # convenient for testing
        self.homeWin.linePort.setText('9998')
        self.homeWin.lineEdit.returnPressed.connect(self.login)
        self.homeWin.pushButton.clicked.connect(self.login)

    '''
    connects FTP server and logs in successively
    '''
    def login(self):
        login_res = self.client.new_connect(self.homeWin.lineIP.text(), int(self.homeWin.linePort.text()))
        if login_res[0]:
            if self.client.login(self.homeWin.user.text(), self.homeWin.lineEdit.text()):
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
        self.funcWin.pasvBtn.setChecked(True)

    '''
    set PORT mode
    '''
    def set_port(self):
        self.client.port_mode()
        self.funcWin.pasvBtn.setChecked(False)
        self.funcWin.portBtn.setChecked(True)

    '''
    sets TYPE through menubar, though it is useless since default is already binary mode
    '''
    def set_type(self):
        if self.downloading or self.uploading:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        self.funcWin.actionSet_binary_mode.setText('Set binary mode √')
        self.client.type_cmd()

    '''
    checks SYST info
    '''
    def ask_syst(self):
        if self.downloading or self.uploading:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        self.client.syst_cmd()

    '''
    attempts to close connection, close current window, and show a result window
    '''
    def close_connect(self, resp):
        # is there is an ongoing transmission, ignore this attempt, pause the transmission and then close again
        if resp == 'ask':
            doClose = QMessageBox.information(None, 'Warning', 'Still tranfering, do you still want to leave?', QMessageBox.Yes|QMessageBox.No)
            if doClose == QMessageBox.No:
                return
            else:
                self.closing = True
                if self.downloading:
                    self.close_download_transfer()
                else:
                    self.close_upload_transfer()
        else:
            self.client.quit_cmd()
            self.resultWin = ResultWindow()
            self.resultWin.show()
            self.resultWin.textBrowser.setText(self.client.statistics)
            self.resultWin.textBrowser.moveCursor(self.resultWin.textBrowser.textCursor().End)

    '''
    downloads file without typing directory
    '''
    def download_file(self):
        if self.downloading or self.uploading:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        selected_dir = self.funcWin.treeWidget.currentItem()
        if selected_dir is None or selected_dir.text(0) == '..' or selected_dir.text(1) != 'File':
            QMessageBox.information(None, 'Error', 'Not a file.', QMessageBox.Yes)
            return

        # restore buttons and progress bars
        if self.funcWin.downloadPause.text() == 'Resume':
            self.funcWin.downloadBar.setValue(0)
            self.funcWin.downloadPause.setText('Pause')
            self.funcWin.downloadPause.clicked.disconnect(self.resume_download_transfer)
            self.funcWin.downloadPause.clicked.connect(self.close_download_transfer)
        if self.funcWin.uploadPause.text() == 'Resume':
            self.funcWin.uploadBar.setValue(0)
            self.funcWin.uploadPause.setText('Pause')
            self.funcWin.uploadPause.clicked.disconnect(self.resume_upload_transfer)
            self.funcWin.uploadPause.clicked.connect(self.close_upload_transfer)

        filesize = int(selected_dir.text(3))
        src_path = selected_dir.text(0).strip()
        if self.client.prefix[-1] == '/':
            src_path = self.client.prefix + src_path
        else:
            src_path = self.client.prefix + '/' + src_path
        dest_path = QFileDialog.getExistingDirectory(None, "Choose a folder.", os.getcwd())
        if not dest_path:
            return
        dir = os.path.join(dest_path, os.path.basename(src_path))
        if os.path.exists(dir):
            do_replace = QMessageBox.information(None, 'Wait', 'Would you like to replace this file?',
                                    QMessageBox.Yes | QMessageBox.No)
            if do_replace == QMessageBox.Yes:
                offset = 0
            elif do_replace == QMessageBox.No:
                offset = os.path.getsize(dir)
            else:
                return
        else:
            offset = 0

        # memorizes the last download/upload operation to enable transmission resume
        self.last_dest = dest_path
        self.last_src = src_path
        self.last_size = filesize
        self.last_offset = None

        self.start_download(src_path, dest_path, filesize, offset)

    '''
    starts downloading
    '''
    def start_download(self, src_path, dest_path, filesize, offset):
        self.downloading = True
        self.funcWin.doClose = False
        self.client.download_file(src_path, dest_path, self.funcWin.downloadBar, filesize, offset)

        def download_complete():
            self.downloading = False
            self.funcWin.doClose = True
            if self.closing:
                self.funcWin.finally_close()
                return
            self.last_offset = self.client.download_thread.offset
            self.funcWin.serverResp.setText(self.client.resp)
            self.funcWin.serverResp.moveCursor(self.funcWin.serverResp.textCursor().End)

        self.client.download_thread.complete_signal.connect(download_complete)

    '''
    uploads file without typing directory
    '''
    def upload_file(self):
        if self.downloading or self.uploading:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        selected_dir = self.funcWin.treeWidget.currentItem()
        dest_folder = ''
        if selected_dir is not None:
            dest_folder = selected_dir.text(0)
            if dest_folder == '..' or selected_dir.text(1) != 'Folder':
                QMessageBox.information(None, 'Error', 'Not a folder!', QMessageBox.Yes)
                return

        # restore buttons and progress bars
        if self.funcWin.uploadPause.text() == 'Resume':
            self.funcWin.uploadBar.setValue(0)
            self.funcWin.uploadPause.setText('Pause')
            self.funcWin.uploadPause.clicked.disconnect(self.resume_upload_transfer)
            self.funcWin.uploadPause.clicked.connect(self.close_upload_transfer)
        if self.funcWin.downloadPause.text() == 'Resume':
            self.funcWin.downloadBar.setValue(0)
            self.funcWin.downloadPause.setText('Pause')
            self.funcWin.downloadPause.clicked.disconnect(self.resume_download_transfer)
            self.funcWin.downloadPause.clicked.connect(self.close_download_transfer)

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

        offset = 0

        if dest_folder:
            self.client.cwd_cmd(dest_folder)
        self.funcWin.treeWidget.clear()
        dirs = self.unfold()
        for dir in dirs:
            if dir.text(0) == os.path.basename(src_path):
                do_replace = QMessageBox.information(None, 'Wait', 'Would you like to replace this file?',
                                                     QMessageBox.Yes | QMessageBox.No)
                if do_replace == QMessageBox.Yes:
                    offset = 0
                else:
                    offset = int(dir.text(3))

        # memorizes the last download/upload operation to enable transmission resume
        self.last_dest = dest_path
        self.last_src = src_path
        self.last_size = None

        self.start_upload(src_path, dest_path, offset)

    '''
    starts uploading
    '''
    def start_upload(self, src_path, dest_path, offset):
        self.uploading = True
        self.funcWin.doClose = False
        self.client.upload_file(src_path, dest_path, self.funcWin.uploadBar, offset)

        def upload_complete():
            self.funcWin.doClose = True
            self.uploading = False
            if self.closing:
                self.funcWin.finally_close()
                return
            #self.client.cwd_cmd(self.last_dest)
            self.funcWin.treeWidget.clear()
            dirs = self.unfold()
            for dir in dirs:
                if dir.text(0) == os.path.basename(self.last_src):
                    self.last_offset = int(dir.text(3))

        self.client.upload_thread.complete_signal.connect(upload_complete)

    '''
    use list command to show directory info from the server
    '''
    def unfold(self):
        root = self.funcWin.treeWidget

        flst = self.client.list_cmd(None)
        if flst is None:
            self.set_pasv()
            flst = self.client.list_cmd(None)

        # the root folder doesn't have .. for security issues
        if self.client.prefix != self.client.root:
            ret = QTreeWidgetItem(root)
            ret.setText(0, '..')
            ret.isFile = '..'

        if flst == '' or flst is None:
            return []

        flst = flst.split('\n')[:-1]
        items = []
        for file in flst:
            parsed_info = []
            file = file.split(' ')
            for i in range(len(file)):
                if file[i] != '':
                    parsed_info.append(file[i])
            item = QTreeWidgetItem(root)
            item.setText(0, ' '.join(parsed_info[8:]).strip('\r'))
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
            if self.downloading or self.uploading:
                QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
                return
            root = self.funcWin.treeWidget
            ftp = self.client
            file = root.currentItem()
            path = file.text(0)

            # change a parent directory
            if file.isFile == '..':
                root.clear()
                ftp.cwd_cmd(root.parent[-1])
                ftp.pwd_cmd()
                root.parent = root.parent[:-1]
                self.unfold()


            if file.isFile == 'no':
                root.clear()
                #root.grandparent = root.parent
                root.parent.append(ftp.prefix)
                if ftp.prefix == '/':
                    ftp.cwd_cmd(ftp.prefix + path)
                else:
                    ftp.cwd_cmd(ftp.prefix + '/' + path)
                ftp.pwd_cmd()
                self.unfold()

        root = self.funcWin.treeWidget
        root.setHeaderLabels(['Filename', 'Type', 'Last modified', 'Size(Bytes)'])
        root.doubleClicked.connect(refresh) # doubleclick on folder to enter
        root.expandAll()
        root.parent = []
        self.client.pwd_cmd() # use PWD command to get prefix
        self.client.root = self.client.prefix
        self.unfold()

    '''
    creates a new folder
    '''
    def mkdir(self):
        if self.downloading or self.uploading:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        self.client.mkd_cmd(self.funcWin.newdirLine.text())
        self.funcWin.treeWidget.clear()
        self.unfold()

    '''
    removes a folder recursively or removes a file
    '''
    def rmdir(self):
        if self.downloading or self.uploading:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        ftp = self.client
        root = self.funcWin.treeWidget
        selected_file = root.currentItem()
        if selected_file is None:
            QMessageBox.information(None, 'Error', 'No directory selected!', QMessageBox.Yes)
            return
        path = selected_file.text(0)
        ftp.dele_cmd(path)
        if self.client.code != 250:
            ftp.rmd_cmd(path)
        root.clear()
        self.unfold()

    '''
    removes a folder recursively or removes a file
    '''
    def rename_file(self):
        if self.downloading or self.uploading:
            QMessageBox.information(None, 'Error', 'File transfering!', QMessageBox.Yes)
            return
        ftp = self.client
        root = self.funcWin.treeWidget
        selected_file = root.currentItem()
        if selected_file is None:
            QMessageBox.information(None, 'Error', 'No file selected!', QMessageBox.Yes)
            return
        path = selected_file.text(0)
        ftp.rnfr_cmd(path)
        if ftp.code != 350:
            return
        newpath = self.funcWin.renameLine.text()
        ftp.rnto_cmd(newpath)
        root.clear()
        self.unfold()

    '''
    pauses a ongoing file upload
    '''
    def close_upload_transfer(self):
        if not self.uploading:
            QMessageBox.information(None, 'Error', 'No file uploading!', QMessageBox.Yes)
            return

        self.funcWin.uploadPause.setText("Resume")
        self.funcWin.uploadPause.clicked.disconnect(self.close_upload_transfer)
        self.funcWin.uploadPause.clicked.connect(self.resume_upload_transfer)
        self.client.data_s.close()

    '''
    pauses a ongoing file download
    '''
    def close_download_transfer(self):
        if not self.downloading:
            QMessageBox.information(None, 'Error', 'No file downloading!', QMessageBox.Yes)
            return
        self.funcWin.downloadPause.setText("Resume")
        self.funcWin.downloadPause.clicked.disconnect(self.close_download_transfer)
        self.funcWin.downloadPause.clicked.connect(self.resume_download_transfer)
        self.client.data_s.close()
        #self.client.abor_cmd()

    '''
    switches from pause to resume
    '''
    def resume_download_transfer(self):
        self.funcWin.downloadPause.setText("Pause")
        self.start_download(self.last_src, self.last_dest, self.last_size, self.last_offset)
        self.funcWin.downloadPause.clicked.disconnect(self.resume_download_transfer)
        self.funcWin.downloadPause.clicked.connect(self.close_download_transfer)

    '''
    switches from pause to resume
    '''
    def resume_upload_transfer(self):
        self.funcWin.uploadPause.setText("Pause")

        self.start_upload(self.last_src, self.last_dest, self.last_offset)
        self.funcWin.uploadPause.clicked.disconnect(self.resume_upload_transfer)
        self.funcWin.uploadPause.clicked.connect(self.close_upload_transfer)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    ftpClient = FTPClient()
    sys.exit(app.exec_())
