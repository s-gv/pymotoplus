# Copyright (c) 2020 Sagar Gubbi. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import pymotoplus as pmp

def main():
    # move to start position just above hole
    pmp.moveL(px=561*1000, py=110*1000, pz=-264*1000, rx=180*10000, ry=0, rz=0, v=500)

if __name__ == "__main__":
    main()
