#!/usr/bin/env python

import sys
from threading import Thread

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from ui_conv2 import Ui_Conv2
import popen2
import threading

exe  = "..\\bin\\conv2.exe"
dict = "..\\dict" 

def writer(stream, data):
	for d in data:
		stream.write(d)
		stream.flush()
	stream.close()

class Conv2(QWidget):
	def __init__(self, parent = None):
		QWidget.__init__(self, parent)
		self.ui = Ui_Conv2()
		self.ui.setupUi(self)
		
		QObject.connect(self.ui.pushButton, SIGNAL("clicked(bool)"), self.process)

	def process(self):
		cmd = "%s --dpath %s" % (exe, dict)

		self.ui.errors.clear()
		self.ui.output.clear()

		cout, cin, cerr = popen2.popen3(cmd, mode = 'b')

		s = self.ui.input.toPlainText()
		s = unicode(s).encode("utf-8")

		w = threading.Thread(target = writer, args = [cin, s])
		w.start()

		while True:			
			try:
				txt = unicode(cout.next(), "utf-8")
				self.ui.output.appendPlainText(txt)
			except StopIteration:
				break

			try:
				err = unicode(cerr.next(), "utf-8")
				self.ui.errors.appendPlainText(txt)
			except StopIteration:
				pass

		w.join()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = Conv2()
    window.show()
    sys.exit(app.exec_())
