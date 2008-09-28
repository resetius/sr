#!/usr/bin/env python

import sys
from threading import Thread

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from ui_conv2 import Ui_Conv2
import popen2
import threading

exe  = "..\\bin\\conv2.exe"
dict = "../dict/" 

class Conv2(QWidget):
	def __init__(self, parent = None):
		QWidget.__init__(self, parent)
		self.ui = Ui_Conv2()
		self.ui.setupUi(self)
		
		QObject.connect(self.ui.pushButton, SIGNAL("clicked(bool)"), self.process)

	def process(self):
		self.ui.errors.clear()
		self.ui.output.clear()

		s = self.ui.input.toPlainText()
		s = unicode(s).encode("utf-8")

		self.p = QProcess()

		QObject.connect(self.p, SIGNAL("readyReadStandardOutput()"), self.write_output)
		QObject.connect(self.p, SIGNAL("readyReadStandardError()"), self.write_errors)

		self.output = ""
		self.p.start(exe, ["--dpath", dict])
		self.p.waitForStarted()
		self.p.write(s)
		self.p.closeWriteChannel()
		self.p.waitForFinished()

		self.ui.output.setPlainText(self.output)

	def write_output(self):
		data = self.p.readAllStandardOutput()
		text = QString.fromUtf8(data.data())
		self.ui.output.appendPlainText(text)
		self.ui.output.moveCursor(QTextCursor.End)
		self.repaint()

		self.output += text

	def write_errors(self):
		data = self.p.readAllStandardError()
		text = QString.fromUtf8(data.data())
		self.ui.errors.appendPlainText(text)
		self.ui.errors.moveCursor(QTextCursor.End)
		self.repaint()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = Conv2()
    window.show()
    sys.exit(app.exec_())
