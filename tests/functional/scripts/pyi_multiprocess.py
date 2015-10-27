#-----------------------------------------------------------------------------
# Copyright (c) 2005-2016, PyInstaller Development Team.
#
# Distributed under the terms of the GNU General Public License with exception
# for distributing bootloader.
#
# The full license is in the file COPYING.txt, distributed with this software.
#-----------------------------------------------------------------------------


# Bootloader unsets _MEIPASS2 for child processes to allow running
# PyInstaller binaries inside pyinstaller binaries.
# This is ok for mac or unix with fork() system call.
# But on Windows we need to overcome missing fork() fuction for onefile
# mode.
#
# See http://www.pyinstaller.org/wiki/Recipe/Multiprocessing


import multiprocessing
import sys


class SendeventProcess(multiprocessing.Process):
    def __init__(self, result_queue):
        self.result_queue = result_queue

        multiprocessing.Process.__init__(self)
        self.start()

    def run(self):
        print('SendeventProcess')
        self.result_queue.put((1, 2))
        print('SendeventProcess')


if __name__ == '__main__':
    # On Windows calling this function is necessary.
    if sys.platform.startswith('win'):
        multiprocessing.freeze_support()
    print('main')
    result_queue = multiprocessing.Queue()
    SendeventProcess(result_queue)
    print('main')
