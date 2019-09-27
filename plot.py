# Copyright (c) 2019 Sagar Gubbi. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import numpy as np
import matplotlib
import json
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def main():
    rows = []
    with open('log.txt') as f:
        for line in f:
            if line.startswith(':u_fx:'):
                txt = '{"' + line[1:].strip().replace(': ', '": ').replace(', ', ', "') + '}'
                row = json.loads(txt)
                if row['rx'] > 0: row['rx'] -= 360*10000
                rows.append(row)
            
    prop_names = ['u_fx', 'u_fy', 'u_fz', 'u_frx', 'u_fry', 'u_frz', 'pz', 'rx', 'ry', 'fx', 'fy', 'fz', 'frx', 'fry']

    fig, axes = plt.subplots(len(prop_names))

    for i, prop_name in enumerate(prop_names):
        axes[i].plot(range(len(rows)), np.array([row[prop_name] for row in rows]))
        axes[i].yaxis.set_label_position("right")
        axes[i].set_ylabel(prop_name, rotation=0, labelpad=20)
        

    fig.set_size_inches(7.5, 10)
    fig.savefig('fig.png')

if __name__ == '__main__':
    main()

