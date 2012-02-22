from __future__ import division
import sys
from matplotlib.pyplot import *
import numpy
import json

def main(filename):
    with open(filename) as f:
        data = json.load(f)

    # plot the graph and save it
    for testname, results in data.items():
        suptitle(testname)
        ylabel('speed (operations/second)')
        xlabel('number of operations')

        def show(data, *args, **kwargs):
            xs = [x for x, y in data]
            ys = [x/y for x, y in data]
            plot(xs, ys, *args, **kwargs)

        show(results['DenseTable'], '-o', color='#cccccc', label='dense_hash_map (open addressing)')
        show(results['OpenTable'], 'b-o', label='open addressing')
        show(results['CloseTable'], 'r-o', label='Close table')
        legend(loc='best')
        savefig(testname + "-speed.png", format='png')
        clf()

main(sys.argv[1])
