# ----------------------------------------------------------------------------
# Copyright (c) 2015-2016, PyInstaller Development Team.
#
# Distributed under the terms of the GNU General Public License with exception
# for distributing bootloader.
#
# The full license is in the file COPYING.txt, distributed with this software.
# ----------------------------------------------------------------------------

from PyInstaller.utils.hooks import collect_data_files
import os

# Auxiliary data for isoschematron
datas = collect_data_files('lxml', subdir=os.path.join('isoschematron', 'resources'))
