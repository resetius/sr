# -*- coding: utf-8 -*-

from popen2 import popen2

class StemResult:
    def __init__(self, data):
        if 'txt_from' in data:
            self.txt_from = data['txt_from']
            cmd = "/home/manwe/projects/sr/web/1.sh"
            sin, sout = popen2(cmd)
            sout.write(self.txt_from.encode("utf-8"))
            sout.close()

            self.txt_to = ""
            for i in sin:
                self.txt_to += unicode(i, "utf-8")

        elif 'txt_to' in data:
            self.txt_to = data['txt_to']
        else:
            self.txt_from = ""
            self.txt_to   = ""

