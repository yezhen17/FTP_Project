# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'func.ui'
#
# Created by: PyQt5 UI code generator 5.13.0
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_func(object):
    def setupUi(self, func):
        func.setObjectName("func")
        func.resize(642, 811)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(func.sizePolicy().hasHeightForWidth())
        func.setSizePolicy(sizePolicy)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        func.setFont(font)
        self.centralwidget = QtWidgets.QWidget(func)
        self.centralwidget.setObjectName("centralwidget")
        self.verticalLayoutWidget = QtWidgets.QWidget(self.centralwidget)
        self.verticalLayoutWidget.setGeometry(QtCore.QRect(9, 9, 621, 741))
        self.verticalLayoutWidget.setObjectName("verticalLayoutWidget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.verticalLayoutWidget)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.pasvBtn = QtWidgets.QRadioButton(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.pasvBtn.setFont(font)
        self.pasvBtn.setObjectName("pasvBtn")
        self.horizontalLayout.addWidget(self.pasvBtn)
        self.portBtn = QtWidgets.QRadioButton(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.portBtn.setFont(font)
        self.portBtn.setObjectName("portBtn")
        self.horizontalLayout.addWidget(self.portBtn)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.serverResp = QtWidgets.QTextBrowser(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        font.setBold(False)
        font.setWeight(50)
        self.serverResp.setFont(font)
        self.serverResp.setObjectName("serverResp")
        self.verticalLayout.addWidget(self.serverResp)
        self.treeWidget = QtWidgets.QTreeWidget(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.treeWidget.setFont(font)
        self.treeWidget.setAutoFillBackground(False)
        self.treeWidget.setIndentation(0)
        self.treeWidget.setObjectName("treeWidget")
        self.treeWidget.headerItem().setText(0, "1")
        self.verticalLayout.addWidget(self.treeWidget)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setSizeConstraint(QtWidgets.QLayout.SetDefaultConstraint)
        self.horizontalLayout_2.setContentsMargins(-1, -1, -1, 0)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.uploadButton = QtWidgets.QPushButton(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.uploadButton.setFont(font)
        self.uploadButton.setObjectName("uploadButton")
        self.horizontalLayout_2.addWidget(self.uploadButton)
        self.downloadBtn = QtWidgets.QPushButton(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.downloadBtn.setFont(font)
        self.downloadBtn.setObjectName("downloadBtn")
        self.horizontalLayout_2.addWidget(self.downloadBtn)
        self.stopButton = QtWidgets.QPushButton(self.verticalLayoutWidget)
        self.stopButton.setObjectName("stopButton")
        self.horizontalLayout_2.addWidget(self.stopButton)
        self.verticalLayout.addLayout(self.horizontalLayout_2)
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_3.setContentsMargins(-1, -1, -1, 0)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.removeBtn = QtWidgets.QPushButton(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        font.setBold(False)
        font.setWeight(50)
        self.removeBtn.setFont(font)
        self.removeBtn.setObjectName("removeBtn")
        self.horizontalLayout_3.addWidget(self.removeBtn)
        self.line = QtWidgets.QFrame(self.verticalLayoutWidget)
        self.line.setLineWidth(3)
        self.line.setFrameShape(QtWidgets.QFrame.VLine)
        self.line.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.line.setObjectName("line")
        self.horizontalLayout_3.addWidget(self.line)
        self.newdirLine = QtWidgets.QLineEdit(self.verticalLayoutWidget)
        self.newdirLine.setObjectName("newdirLine")
        self.horizontalLayout_3.addWidget(self.newdirLine)
        self.mkdirBtn = QtWidgets.QPushButton(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.mkdirBtn.setFont(font)
        self.mkdirBtn.setObjectName("mkdirBtn")
        self.horizontalLayout_3.addWidget(self.mkdirBtn)
        self.line_2 = QtWidgets.QFrame(self.verticalLayoutWidget)
        self.line_2.setLineWidth(3)
        self.line_2.setFrameShape(QtWidgets.QFrame.VLine)
        self.line_2.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.line_2.setObjectName("line_2")
        self.horizontalLayout_3.addWidget(self.line_2)
        self.renameLine = QtWidgets.QLineEdit(self.verticalLayoutWidget)
        self.renameLine.setObjectName("renameLine")
        self.horizontalLayout_3.addWidget(self.renameLine)
        self.renameBtn = QtWidgets.QPushButton(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.renameBtn.setFont(font)
        self.renameBtn.setObjectName("renameBtn")
        self.horizontalLayout_3.addWidget(self.renameBtn)
        self.verticalLayout.addLayout(self.horizontalLayout_3)
        self.downloadBar = QtWidgets.QProgressBar(self.verticalLayoutWidget)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.downloadBar.setFont(font)
        self.downloadBar.setProperty("value", 0)
        self.downloadBar.setObjectName("downloadBar")
        self.verticalLayout.addWidget(self.downloadBar)
        func.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(func)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 642, 30))
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.menubar.setFont(font)
        self.menubar.setObjectName("menubar")
        self.menuSettings = QtWidgets.QMenu(self.menubar)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.menuSettings.setFont(font)
        self.menuSettings.setObjectName("menuSettings")
        func.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(func)
        self.statusbar.setObjectName("statusbar")
        func.setStatusBar(self.statusbar)
        self.actionSet_binary_mode = QtWidgets.QAction(func)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.actionSet_binary_mode.setFont(font)
        self.actionSet_binary_mode.setObjectName("actionSet_binary_mode")
        self.actionSystem_Info = QtWidgets.QAction(func)
        font = QtGui.QFont()
        font.setFamily("Comic Sans MS")
        font.setPointSize(10)
        self.actionSystem_Info.setFont(font)
        self.actionSystem_Info.setObjectName("actionSystem_Info")
        self.menuSettings.addAction(self.actionSet_binary_mode)
        self.menuSettings.addAction(self.actionSystem_Info)
        self.menubar.addAction(self.menuSettings.menuAction())

        self.retranslateUi(func)
        QtCore.QMetaObject.connectSlotsByName(func)

    def retranslateUi(self, func):
        _translate = QtCore.QCoreApplication.translate
        func.setWindowTitle(_translate("func", "FTP Client"))
        self.pasvBtn.setText(_translate("func", "PASV MODE"))
        self.portBtn.setText(_translate("func", "PORT MODE"))
        self.uploadButton.setText(_translate("func", "Upload"))
        self.downloadBtn.setText(_translate("func", "download"))
        self.stopButton.setText(_translate("func", "Stop"))
        self.removeBtn.setText(_translate("func", "Remove"))
        self.mkdirBtn.setText(_translate("func", "Create new folder"))
        self.renameBtn.setText(_translate("func", "Rename"))
        self.menuSettings.setTitle(_translate("func", "Settings"))
        self.actionSet_binary_mode.setText(_translate("func", "Set binary mode"))
        self.actionSystem_Info.setText(_translate("func", "System Info"))
