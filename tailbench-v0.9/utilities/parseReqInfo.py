#!/usr/bin/python

import sys
import os
import numpy as np
from scipy import stats
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-n', '--normalize', help="normalized time stamp to that of first generated request", action="store_true")
args = parser.parse_args()

class Lat(object):
    def __init__(self, fileName):
        f = open(fileName, 'rb')
        a = np.fromfile(f, dtype=np.uint64)
        self.reqTimes = a.reshape((a.shape[0]/6, 6))
        f.close()

    def parseGenTimes(self):
        return self.reqTimes[:, 0]

    def parseQueue1Times(self):
        return self.reqTimes[:, 1]

    def parseStartTimes(self):
        return self.reqTimes[:, 2]

    def parseSvcTimes(self):
        return self.reqTimes[:, 3]

    def parseQueue2Times(self):
        return self.reqTimes[:, 4]

    def parseTids(self):
        return self.reqTimes[:, 5]

if __name__ == '__main__':
    def getLatPct(latsFile):
        assert os.path.exists(latsFile)

        latsObj = Lat(latsFile)

        genTimes = [l for l in latsObj.parseGenTimes()]
        queue1Times = [l/1e6 for l in latsObj.parseQueue1Times()]
        startTimes = [l for l in latsObj.parseStartTimes()]
        svcTimes = [l/1e6 for l in latsObj.parseSvcTimes()]
        queue2Times = [l/1e6 for l in latsObj.parseQueue2Times()]
        tids = [l for l in latsObj.parseTids()]
        if args.normalize:
            minGenTime = min(genTimes)
            genTimes -= minGenTime
            startTimes -= minGenTime
        f = open('reqInfo.txt','w')
        for (gen, q1, start, svc, q2, tid) in zip(genTimes, queue1Times, startTimes, svcTimes, queue2Times, tids):
            f.write("%d,%.3f,%d,%.3f,%.3f,%d\n" % (gen, q1, start, svc, q2, tid))
        f.close()

    latsFile = sys.argv[1]
    getLatPct(latsFile)

