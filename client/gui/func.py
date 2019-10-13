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
        func.resize(800, 600)
        self.centralwidget = QtWidgets.QWidget(func)
        self.centralwidget.setObjectName("centralwidget")
        self.verticalLayoutWidget = QtWidgets.QWidget(self.centralwidget)
        self.verticalLayoutWidget.setGeometry(QtCore.QRect(9, 9, 781, 531))
        self.verticalLayoutWidget.setObjectName("verticalLayoutWidget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.verticalLayoutWidget)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.pasvBtn = QtWidgets.QRadioButton(self.verticalLayoutWidget)
        self.pasvBtn.setObjectName("pasvBtn")
        self.horizontalLayout.addWidget(self.pasvBtn)
        self.portBtn = QtWidgets.QRadioButton(self.verticalLayoutWidget)
        self.portBtn.setObjectName("portBtn")
        self.horizontalLayout.addWidget(self.portBtn)
        self.verticalLayout.addLayout(self.horizontalLayout)
        spacerItem = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        self.verticalLayout.addItem(spacerItem)
        self.serverResp = QtWidgets.QTextBrowser(self.verticalLayoutWidget)
        self.serverResp.setObjectName("serverResp")
        self.verticalLayout.addWidget(self.serverResp)
        spacerItem1 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem1)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setSizeConstraint(QtWidgets.QLayout.SetDefaultConstraint)
        self.horizontalLayout_2.setContentsMargins(-1, -1, -1, 0)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.dir_btn = QtWidgets.QPushButton(self.verticalLayoutWidget)
        self.dir_btn.setObjectName("dir_btn")
        self.horizontalLayout_2.addWidget(self.dir_btn)
        self.downloadBtn = QtWidgets.QPushButton(self.verticalLayoutWidget)
        self.downloadBtn.setObjectName("downloadBtn")
        self.horizontalLayout_2.addWidget(self.downloadBtn)
        self.verticalLayout.addLayout(self.horizontalLayout_2)
        func.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(func)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 800, 26))
        self.menubar.setObjectName("menubar")
        self.menuSettings = QtWidgets.QMenu(self.menubar)
        self.menuSettings.setObjectName("menuSettings")
        func.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(func)
        self.statusbar.setObjectName("statusbar")
        func.setStatusBar(self.statusbar)
        self.actionSet_binary_mode = QtWidgets.QAction(func)
        self.actionSet_binary_mode.setObjectName("actionSet_binary_mode")
        self.actionSystem_Info = QtWidgets.QAction(func)
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
        self.dir_btn.setText(_translate("func", "Choose a directory"))
        self.downloadBtn.setText(_translate("func", "download"))
        self.menuSettings.setTitle(_translate("func", "Settings"))
        self.actionSet_binary_mode.setText(_translate("func", "Set binary mode"))
        self.actionSystem_Info.setText(_translate("func", "System Info"))
