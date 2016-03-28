#!/usr/bin/python
# ----------------------------------------------------------------------------
# Copyright (c) 2013-2016, PyInstaller Development Team.
#
# Distributed under the terms of the GNU General Public License with exception
# for distributing bootloader.
#
# The full license is in the file COPYING.txt, distributed with this software.
# ----------------------------------------------------------------------------
#
# Tkinter interface to PyInstaller.
#


import subprocess
import sys

# In Python 3 module name is 'tkinter'
try:
    from tkinter import IntVar, Tk
    from tkinter.ttk import Button, Checkbutton, Frame, Entry
    import tkinter.filedialog as filedialog
except ImportError:
    from Tkinter import IntVar, Tk
    from ttk import Button, Checkbutton, Frame, Entry
    import tkFileDialog as filedialog


class PyInstallerGUI:

    @staticmethod
    def make_checkbutton(frame, text):
        var = IntVar()
        widget = Checkbutton(frame, text=text, variable=var)
        widget.grid(sticky="NW")
        return var

    def __init__(self):
        root = Tk()
        root.title("PyInstaller GUI")
        fr1 = Frame(root, width=300, height=100)
        fr1.pack(side="top")

        fr2 = Frame(root, width=300, height=300,
                    borderwidth=2, relief="ridge")
        fr2.pack(ipadx=10, ipady=10)
        fr4 = Frame(root, width=300, height=100)
        fr4.pack(side="bottom", pady=10)

        get_file_button = Button(fr1, text="Script to bundle ...")
        get_file_button.bind("<Button>", self.get_file)
        get_file_button.pack(side="left")
        self.filein = Entry(fr1)
        self.filein.pack(side="right")
        self.filetype = self.make_checkbutton(fr2, "One File Package")
        self.ascii = self.make_checkbutton(fr2, "Do NOT include decodings")
        self.debug = self.make_checkbutton(fr2, "Use debug versions")
        if sys.platform.startswith('win'):
            self.noconsole = self.make_checkbutton(fr2, "No console (Windows only)")
        else:
            self.noconsole = IntVar()
        if not sys.platform.startswith('win'):
            self.strip = self.make_checkbutton(fr2, "Strip the exe and shared libs")
        else:
            self.strip = IntVar()

        okaybutton = Button(fr4, text="Okay   ")
        okaybutton.bind("<Button>", self.make_package)
        okaybutton.pack(side="left")

        cancelbutton = Button(fr4, text="Cancel")
        cancelbutton.bind("<Button>", self.killapp)
        cancelbutton.pack(side="right")
        self.fin = ''

        width_of_screen = root.winfo_screenwidth()
        height_of_screen = root.winfo_screenheight()
        x_coord = (width_of_screen/2) - (400/2)
        y_coord = (height_of_screen/2) - (250/2)
        root.geometry('%dx%d+%d+%d' % (400, 250, x_coord, y_coord))

        root.mainloop()

    @staticmethod
    def killapp(event):
        sys.exit(0)

    def make_package(self, event):
        commands = [sys.executable, 'pyinstaller.py']
        if self.filetype.get():
            commands.append('--onefile')
        if self.ascii.get():
            commands.append('--ascii')
        if self.debug.get():
            commands.append('--debug')
        if self.noconsole.get():
            commands.append('--noconsole')
        if self.strip.get():
            commands.append('--strip')
        commands.append(self.fin)
        retcode = subprocess.call(commands)
        sys.exit(retcode)

    def get_file(self, event):
        self.fin = filedialog.askopenfilename()
        self.filein.delete(0, 'end')
        self.filein.insert(0, self.fin)

if __name__ == "__main__":
    raise SystemExit("Please use just 'pyinstaller.py'. Gui is not maintained.")
    try:
        app = PyInstallerGUI()
    except KeyboardInterrupt:
        raise SystemExit("Aborted by user request.")
