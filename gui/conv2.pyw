#!/usr/bin/env python

import sys

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from ui_conv2 import Ui_Conv2

class Conv2(QWidget):
	def __init__(self, parent = None):
		QWidget.__init__(self, parent)
		self.ui = Ui_Conv2()
		self.ui.setupUi(self)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = Conv2()
    window.show()
    sys.exit(app.exec_())
