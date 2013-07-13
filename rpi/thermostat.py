#!/usr/bin/env python

from collections import defaultdict
import json
import urllib2
import sys
import datetime
import time

HOSTNAMES = {
    'thermostat.downstairs': '192.168.0.201',
    'thermostat.upstairs': '192.168.0.202'
}


def main():
    all_data = defaultdict(dict)

    for dataset, hostname in HOSTNAMES.items():
        url = 'http://%s/tstat/eventlog' % hostname
        stream = urllib2.urlopen(url)
        if stream.getcode() != 200:
            print 'Request to %s returned %d' % (url, data.getcode())
            sys.exit(1)
        data = json.load(stream)
        # {"eventlog":[["hour","minute","relay","temp","humidity","ttemp"],[0,20,256,72.00,40,76]]}

        eventlog = data['eventlog']
        headers = eventlog.pop(0)
        for point in eventlog:
            d = {}
            hour = None
            minute = None
            for i, name in enumerate(headers):
                if name == 'hour':
                    hour = point[i]
                elif name == 'minute':
                    minute = point[i]
                else:
                    d[name] = point[i]

            if hour < 23 or minute < 50:
                tstamp = datetime.datetime.now()
                tstamp = tstamp.replace(hour=hour).replace(minute=minute).replace(second=0)
                tstamp = tstamp.replace(microsecond=0)
                # Convert to UTC (broken?)
#                key = str(datetime.datetime.fromtimestamp(time.mktime(time.gmtime(time.mktime(tstamp.timetuple())))))
                key = str(tstamp)
                all_data[dataset][key] = d

    print 'Uploading data: %s' % json.dumps(all_data)
    req = urllib2.Request('http://home-journal.appspot.com/api/add-data-point',
                          json.dumps(all_data),
                          {'User-Agent' : 'thermostat/0.1'})
    resp = urllib2.urlopen(req)
    if resp.getcode() != 200:
        print 'Uploading %s failed: %d' % (json.dumps(all_data), resp.getcode())


if __name__ == "__main__":
    main()
