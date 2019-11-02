# Copyright (c) 2019 Sagar Gubbi. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import socket
import time
import sys
import json
import random

CLIENT_UDP_IP = "0.0.0.0"
SERVER_UDP_IP = "192.168.255.1"
UDP_PORT = 22000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((CLIENT_UDP_IP, UDP_PORT))

def send(msg):
    sock.sendto(bytes(msg, 'utf-8'), (SERVER_UDP_IP, UDP_PORT))

f = open('log.txt', 'w')

def wait(log=True, terminal_print=True):
    while True:
        data, addr = sock.recvfrom(1024)
        txt = data.decode('utf-8').strip()
        if terminal_print:
            print(txt)
        if log:
            f.write(txt+'\n')
            #f.flush()
        if txt.startswith(':'):
            return txt
        elif txt.startswith('!'):
            print(txt)
            raise ValueError(txt)

def to_json(txt):
    json_txt = '{"' + txt[1:].strip().replace(': ', '": ').replace(', ', ', "') + '}'
    data = json.loads(json_txt)
    return data

def initForceMeasure(f_avg_time=10):
    send(f':A {f_avg_time}')
    wait()

def startImpControl(m=1, mr=None, d=3000, dr=None, dx=None, dy=None, dz=None, drx=None, dry=None, drz=None, k=0, kr=None, axes=63, contact=1):
    if mr is None: mr = m
    if dr is None: dr = d
    if kr is None: kr = k
    
    if dx is None: dx = d
    if dy is None: dy = d
    if dz is None: dz = d

    if drx is None: drx = dr
    if dry is None: dry = dr
    if drz is None: drz = dr
    
    send(f':B {m} {mr} {dx} {dy} {dz} {drx} {dry} {drz} {k} {kr} {axes} {contact}')
    wait()

def endImpControl():
    send(':C')
    wait()

def setForce(fx=0, fy=0, fz=-10, frx=0, fry=0, frz=0, terminal_print=False):
    send(f':D {fx} {fy} {fz} {frx} {fry} {frz}')
    wait(terminal_print=terminal_print)

def setTool(robot_no=0, tool_no=0):
    send(f':T {robot_no} {tool_no}')
    wait(terminal_print=terminal_print)

def moveL(px=454*1000, py=12*1000, pz=-221*1000, rx=180*10000, ry=0, rz=0, v=500, terminal_print=False):
    send(f':L {int(px)} {int(py)} {int(pz)} {int(rx)} {int(ry)} {int(rz)} {int(v)}')
    wait(terminal_print=terminal_print)

def measureForcePosition(terminal_print=False):
    send(':E')
    txt = wait(terminal_print=terminal_print)
    json_txt = '{"' + txt[1:].strip().replace(': ', '": ').replace(', ', ', "') + '}'
    data = json.loads(json_txt)
    if data['rx'] > 0: data['rx'] -= 360*10000
    return data

def measureMotoFitData(terminal_print=False):
    initForceMeasure(f_avg_time=100)
    startImpControl(dr=15000, axes=((1 << 2) | (1 << 1) | (1 << 0)))
    setForce(fz=-10)
    for it in range(120):
        since = time.perf_counter()
        data = measureForcePosition(terminal_print=True)
        time.sleep(0.1)
        now = time.perf_counter()
        print(now - since)
    endImpControl()
    send(':F')
    txt = wait(terminal_print=terminal_print)
    data = to_json(txt)
    if data['rx'] > 0: data['rx'] -= 360*10000
    return data

def setVar(var_type='B', idx=0, val=0, terminal_print=False):
    send(f':V {var_type} {idx} {val}')
    wait(terminal_print=terminal_print)

def getVar(var_type='B', idx=0, terminal_print=False):
    send(f':G {var_type} {idx}')
    txt = wait(terminal_print=terminal_print)
    data = to_json(txt)
    return data

def startJob(job_name='TIMER', terminal_print=False):
    send(f':P {job_name}')
    wait(terminal_print=terminal_print)

def startBgJob(terminal_print=False):
    time.sleep(0.5)
    startJob('BG', terminal_print=terminal_print)

def endBgJob(terminal_print=False):
    setVar('B', 0, 0, terminal_print=terminal_print)
    time.sleep(1)

def echo():
    send(':Z')
    wait()


def main():
    # lift peg up
    startBgJob()
    initForceMeasure(f_avg_time=100)
    startImpControl(m=1, dx=8000, dy=8000, dz=8000, drx=8000, dry=8000, drz=8000, axes=31, contact=1)
    setForce(fx=0, fy=0, fz=20)
    time.sleep(4)
    endImpControl()
    endBgJob()

    # move to start position just above hole
    moveL(px=561*1000, py=110*1000, pz=-264*1000, rx=180*10000, ry=0, rz=0, v=500)

if __name__ == "__main__":
    main()

