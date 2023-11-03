#!/usr/bin/python

import sys
import os
import numpy as np
from scipy import stats

class Lat(object):
    def __init__(self, fileName):
        f = open(fileName, 'rb')
        self.reqTimes = np.fromfile(f, dtype=np.uint64)
        f.close()

    def parseSojournTimes(self):
        return self.reqTimes

if __name__ == '__main__':
    def getLatPct(latsFile):
        assert os.path.exists(latsFile)

        latsObj = Lat(latsFile)
        sjrnTimes = [l/1e6 for l in latsObj.parseSojournTimes()]
        p95 = stats.scoreatpercentile(sjrnTimes, 95)
        maxLat = max(sjrnTimes)
        print "95th percentile latency %.3f ms | max latency %.3f ms" \
                % (p95, maxLat)

    latsFile = sys.argv[1]
    getLatPct(latsFile)
