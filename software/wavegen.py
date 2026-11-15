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
import serial
import time
import types

from datetime import datetime

import serial.tools.list_ports

ref_freq = 25.000E6
max_freq = 10E6
min_freq = 0
max_gain = 4095
min_gain = 0
max_offset = 1023
min_offset = 0

message = None
port = None

#
def serial_write(self, data):
    port_write(data)

    if (message is not None):
        ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        message.append('%s: send: %s' % (ts, data)) 

#
def serial_readline(self):
    data = port_readline()

    if (message is not None):
        ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        message.append('%s: rcvd: %s' % (ts, data)) 

    return data

#
def wait():
    start_time = time.time()
    timeout_expiry = start_time+1.0

    while time.time() < timeout_expiry:
        value = port.readline().decode().strip('\n')

        if (value != ''):
            return value

#
    return False

#
def set_frequency(freq):
    if (wait() != 'READY'):
        return -1

    data = 'freq:%d:\n' % (freq)
    port.write(data.encode())

    while (wait() != 'DONE'):
        continue

#
    return 0

#
def set_frequency_hz(freq):
    if (wait() != 'READY'):
        return -1

    if (freq > max_freq):
        freq = max_freq

    elif (freq < min_freq):
        freq = min_freq

    data = 'freq-hz:%d:\n' % (freq)
    port.write(data.encode())

    while (wait() != 'DONE'):
        continue

#
    return 0

#
def get_frequency():
    if (wait() != 'READY'):
        return -1
    
    data = 'freq?:\n'
    port.write(data.encode())
    
    freq = port.readline().decode().strip('\n')
    freq = int(freq)

    while (wait() != 'DONE'):
        continue

#
    return freq

#
def set_gain(gain):
    if (wait() != 'READY'):
        return -1

    if (gain > max_gain):
        gain = max_gain

    elif (gain < min_gain):
        gain = min_gain

    data = 'gain:%d:\n' % (gain)
    port.write(data.encode())

    while (wait() != 'DONE'):
        continue

#
    return 0

#  
def get_gain():
    if (wait() != 'READY'):
        return -1
    
    data = 'gain?:\n'
    port.write(data.encode())
    
    gain = port.readline().decode().strip('\n')
    gain = int(gain)

    while (wait() != 'DONE'):
        continue

#
    return gain

#
def set_offset(offset):
    if (wait() != 'READY'):
        return -1

    if (offset > max_offset):
        offset = max_offset

    elif (offset < min_offset):
        offset = min_offset

    data = 'offset:%d:\n' % (offset)
    port.write(data.encode())

    while (wait() != 'DONE'):
        continue

#
    return 0

#  
def get_offset():
    if (wait() != 'READY'):
        return -1
    
    data = 'offset?:\n'
    port.write(data.encode())
    
    offset = port.readline().decode().strip('\n')
    offset = int(offset)

    while (wait() != 'DONE'):
        continue

#
    return offset

#
def set_phase(phase):
    if (wait() != 'READY'):
        return -1

    data = 'phase:%d:\n' % (phase)
    port.write(data.encode())

    while (wait() != 'DONE'):
        continue

#
    return 0

#
def get_phase():
    if (wait() != 'READY'):
        return -1
    
    data = 'phase?:\n'
    port.write(data.encode())
    
    phase = port.readline().decode().strip('\n')
    phase = int(phase)

    while (wait() != 'DONE'):
        continue

#
    return phase

#
def set_waveform(waveform):
    if (wait() != 'READY'):
        return -1

    data = 'wave:%s:\n' % (waveform)
    port.write(data.encode())

    while (wait() != 'DONE'):
        continue

#
    return 0
    
#
def set_output(freq_select, phase_select):
    if (wait() != 'READY'):
        return -1

    data = 'output:%d:%d:\n' % (freq_select, phase_select)
    port.write(data.encode())

    while (wait() != 'DONE'):
        continue

#
    return 0
    
#
def send_fsk_data(count, payload):
    if (wait() != 'READY'):
        return -1
    
    data = 'send-fsk:%d:%s:\n' % (count, payload)
    port.write(data.encode('latin-1'))

    while (wait() != 'DONE'):
        continue

#
    return 0

#
def send_psk_data(count, payload):
    if (wait() != 'READY'):
        return -1
    
    data = 'send-psk:%d:%s:\n' % (count, payload)
    port.write(data.encode('latin-1'))

    while (wait() != 'DONE'):
        continue

#
    return 0

#
def init():
    global port
    global port_write
    global port_readline

    ports = serial.tools.list_ports.comports()

#
    for p in ports:
        if p.manufacturer == None:
            continue

        elif 'Arduino' in p.manufacturer:
            port = serial.Serial(p.device)

            port.baudrate = 57600
            port.bytesize = 8
            port.parity = serial.PARITY_NONE
            port.stopbits = serial.STOPBITS_ONE
            port.timeout = 0.5

            port.flushInput()
            port.flushOutput()

            port_write = port.write
            port.write = types.MethodType(serial_write, port)

            port_readline = port.readline
            port.readline = types.MethodType(serial_readline, port)

            return 0

    return -1

