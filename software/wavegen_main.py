#!/usr/bin/env python3
# Copyright (C) 2026 Noyan Kinayman, KC1VCN
#
# This file is part of the KC1VCN Waveform Generator project.
#
# This source describes Open Hardware and is licensed under the
# CERN Open Hardware Licence Version 2 - Weakly Reciprocal
# (CERN-OHL-W-2.0).
#
# You may redistribute and modify this source and make products
# using it under the terms of CERN-OHL-W-2.0.
#
# This source is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY,
# INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS
# FOR A PARTICULAR PURPOSE. Please see the CERN-OHL-W-2.0 licence
# for applicable conditions.
#
# SPDX-License-Identifier: CERN-OHL-W-2.0
#
import sys
import os
import math
import re
import time
import ast
import random
import argparse
import json
import zlib
import functools

from datetime import datetime
from datetime import timezone

from PySide6 import QtWidgets
from PySide6 import QtCore
from PySide6 import QtGui

from PySide6.QtWidgets import QApplication
from PySide6.QtWidgets import QMainWindow
from PySide6.QtWidgets import QPushButton
from PySide6.QtWidgets import QMessageBox

from PySide6.QtCore import QTimer
from PySide6.QtCore import QTime
from PySide6.QtCore import Qt
from PySide6.QtCore import QCoreApplication

from PySide6.QtGui import QPainter
from PySide6.QtGui import QColor
from PySide6.QtGui import QBrush
from PySide6.QtGui import QFont
from PySide6.QtGui import QPen

import wavegen as wg

from wavegen_ui import Ui_MainWindow

#
class MainWindow(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)

        self.setupUi(self)

        self.setWindowTitle('Waveform Generator')

#  
        self.Frequency_dial.valueChanged.connect(self.frequency_dial)
        self.Frequency_lineEdit.editingFinished.connect(self.frequency_keyboard)
        self.Frequency_radioButton_1.toggled.connect(self.frequency_set_multiplier)
        self.Frequency_radioButton_2.toggled.connect(self.frequency_set_multiplier)
        self.Frequency_radioButton_3.toggled.connect(self.frequency_set_multiplier)
        self.Frequency_radioButton_4.toggled.connect(self.frequency_set_multiplier)

        self.frequency_multiplier = 1

        text_font = QFont('Monospace', 16)
        text_font.setStyleHint(QFont.StyleHint.Monospace) 
        self.Frequency_lineEdit.setFont(text_font)

#
        self.Gain_dial.valueChanged.connect(self.gain_dial)
        self.Gain_lineEdit.editingFinished.connect(self.gain_keyboard)

        text_font = QFont('Monospace', 16)
        text_font.setStyleHint(QFont.StyleHint.Monospace) 
        self.Gain_lineEdit.setFont(text_font)
        
#
        self.Offset_dial.valueChanged.connect(self.offset_dial)
        self.Offset_lineEdit.editingFinished.connect(self.offset_keyboard)

        text_font = QFont('Monospace', 16)
        text_font.setStyleHint(QFont.StyleHint.Monospace) 
        self.Offset_lineEdit.setFont(text_font)

#
        self.Frequency_output_radioButton_1.toggled.connect(self.on_output_changed)
        self.Frequency_output_radioButton_2.toggled.connect(self.on_output_changed)

#
        self.Mod_pushButton_1.clicked.connect(self.on_FSK_button_clicked)
        self.Mod_pushButton_2.clicked.connect(self.on_BPSK_button_clicked)

#
        self.Waveform_comboBox.currentIndexChanged.connect(self.on_waveform_changed)

#
        if (wg.init() != 0):
            print('Error: Can\'t detect Arduino UNO R3.\n')
            sys.exit(1)

#
        time.sleep(5)

        wg.message = self.message

        wg.set_output(1, 1)
        wg.set_frequency(0)
        wg.set_phase(0)

        wg.set_output(0, 0)
        wg.set_frequency(0)
        wg.set_phase(0)

        wg.set_gain(2048)
        wg.set_offset(335)

        self.Frequency_lcdNumber.display(0)
        self.Gain_lcdNumber.display(2048)
        self.Offset_lcdNumber.display(335)

################################
# Frequency
################################
    def frequency_dial(self):
        dial_value = self.Frequency_dial.value()

        stored_dial_value = self.Frequency_dial.property('stored_value')

        step = dial_value-stored_dial_value

        if abs(step) > 50:
            step = (step+99) if step < 0 else (step-99)

        self.Frequency_dial.setProperty('stored_value', dial_value)

#
        freq = int(self.Frequency_lcdNumber.value())+step*self.frequency_multiplier

#
        if freq > wg.max_freq:
            freq = wg.max_freq

        elif freq < wg.min_freq:
            freq = wg.min_freq

#
        freq = round(freq*float(1 << 28)/wg.ref_freq)
        wg.set_frequency(freq)
        
        freq = wg.get_frequency()
        freq = round((freq*wg.ref_freq)/float(1 << 28))

#
        freq = '%d' % (freq)
        self.Frequency_lcdNumber.display(freq)

#
    def frequency_keyboard(self):
        freq = self.Frequency_lineEdit.text()

        if (freq == ''):
            return

#
        if (freq[0] == '+') or (freq[0] == '-'):
            current_freq = int(self.Frequency_lcdNumber.value())
            freq = current_freq+int(freq)

        else:
            freq = int(freq)

#
        if freq > wg.max_freq:
            freq = wg.max_freq
            
        elif freq < wg.min_freq:
            freq = wg.min_freq

#
        freq = round(freq*float(1 << 28)/wg.ref_freq)
        wg.set_frequency(freq)

        freq = wg.get_frequency()
        freq = round(freq*wg.ref_freq/float(1 << 28))

#
        freq = '%d' % (freq)
        self.Frequency_lcdNumber.display(freq)

#
    def frequency_set_multiplier(self):
        button = self.sender()
        
        multiplier = button.property('multiplier')

        self.frequency_multiplier = multiplier
        
################################
# Gain
################################
    def gain_dial(self):
        dial_value = self.Gain_dial.value()

        stored_dial_value = self.Gain_dial.property('stored_value')

        step = dial_value-stored_dial_value

        if abs(step) > 50:
            step = (step+99) if step < 0 else (step-99)

        self.Gain_dial.setProperty('stored_value', dial_value)

#
        gain = int(self.Gain_lcdNumber.value())+step*10

#
        if gain > wg.max_gain:
            gain = wg.max_gain

        elif gain <  wg.min_gain:
            gain = wg.min_gain

#
        wg.set_gain(gain)
        gain = wg.get_gain()

#
        gain = '%d' % (gain)
        self.Gain_lcdNumber.display(gain)
        
#
    def gain_keyboard(self):
        gain = self.Gain_lineEdit.text()

        if (gain == ''):
            return

#
        if (gain[0] == '+') or (gain[0] == '-'):
            current_gain = int(self.Gain_lcdNumber.value())
            gain = current_gain+int(gain)

        else:
            gain = int(gain)

#
        if gain > wg.max_gain:
            gain = wg.max_gain
            
        elif gain <  wg.min_gain:
            gain = wg.min_gain

#
        wg.set_gain(gain)
        gain = wg.get_gain()

#
        gain = '%d' % (gain)
        self.Gain_lcdNumber.display(gain)

################################
# DC Offset
################################
    def offset_dial(self):
        dial_value = self.Offset_dial.value()

        stored_dial_value = self.Offset_dial.property('stored_value')

        step = dial_value-stored_dial_value

        if abs(step) > 50:
            step = (step+99) if step < 0 else (step-99)

        self.Offset_dial.setProperty('stored_value', dial_value)

#
        offset = int(self.Offset_lcdNumber.value())+step

#
        if offset > wg.max_offset:
            offset = wg.max_offset

        elif offset <  wg.min_offset:
            offset = wg.min_offset

#
        wg.set_offset(offset)
        offset = wg.get_offset()

#
        offset = '%d' % (offset)
        self.Offset_lcdNumber.display(offset)
        
#
    def offset_keyboard(self):
        offset = self.Offset_lineEdit.text()

        if (offset == ''):
            return

#
        if (offset[0] == '+') or (offset[0] == '-'):
            current_offset = int(self.Offset_lcdNumber.value())
            offset = current_offset+int(offset)

        else:
            offset = int(offset)

#
        if offset > wg.max_offset:
            offset = wg.max_offset
            
        elif offset <  wg.min_offset:
            offset = wg.min_offset

#
        wg.set_offset(offset)
        offset = wg.get_offset()

#
        offset = '%d' % (offset)
        self.Offset_lcdNumber.display(offset)
        
################################
# Waveform
################################
    def on_waveform_changed(self, index):
        waveform = {0:'sin', 1:'tri', 2:'sqr1', 3:'sqr2', 4:'res'}[index]

        wg.set_waveform(waveform)

################################
# Output Register
################################
    def on_output_changed(self, index):
        button = self.sender()

        output = button.property('reg')

        wg.set_output(output, output)

        freq = wg.get_frequency()
        freq = round((freq*wg.ref_freq)/float(1 << 28))

        freq = '%d' % (freq)
        self.Frequency_lcdNumber.display(freq)

################################
# FSK Modulation Test
################################
    def on_FSK_button_clicked(self):
        button = self.sender()

        wg.set_waveform('sin')
        self.Waveform_comboBox.setCurrentIndex(0)
        self.Frequency_lcdNumber.display('')

        self.statusbar.showMessage('Modulation test is in progress...')
        QApplication.processEvents()

        wg.send_fsk_data(1000, 'Hello World!')

        button.setChecked(False)
        self.Frequency_output_radioButton_1.setChecked(True)
        wg.set_output(0, 0)

        self.statusbar.clearMessage()

################################
# BPSK Modulation Test
################################
    def on_BPSK_button_clicked(self):
        button = self.sender()

        wg.set_waveform('sin')
        self.Waveform_comboBox.setCurrentIndex(0)
        self.Frequency_lcdNumber.display('')

        self.statusbar.showMessage('Modulation test is in progress...')
        QApplication.processEvents()

        wg.send_psk_data(1000, 'Hello World!')

        button.setChecked(False)
        self.Frequency_output_radioButton_1.setChecked(True)
        wg.set_output(0, 0)

        self.statusbar.clearMessage()

#
if __name__ == "__main__":
    app = QtWidgets.QApplication()

#
    window = MainWindow()
    window.statusBar().setFixedHeight(31)

#
    window.show()

    sys.exit(app.exec())

