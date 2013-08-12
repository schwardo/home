#!/usr/bin/env python

import cgi
import datetime
import webapp2
import json
import logging

from google.appengine.ext import ndb
from google.appengine.api import users

class Sample(ndb.Model):
  timestamp = ndb.DateTimeProperty()
  dataset = ndb.StringProperty()
  values = ndb.TextProperty()


class MainPage(webapp2.RequestHandler):
  def get(self):
    self.response.out.write('<html><body>')
    self.response.out.write('</body></html>')

class LoadData(webapp2.RequestHandler):
  def get(self):
    dataset = self.request.get('dataset')
    key = self.request.get('key')
    results = []
    q = Sample.gql('WHERE dataset = :1 ORDER BY timestamp', dataset)
    for point in q.fetch(1000):
      values = json.loads(point.values)
      results.append([str(point.timestamp), values[key]])
    self.response.write(json.dumps(results))


class AddDataPoint(webapp2.RequestHandler):
  def post(self):
    for dataset, data in json.loads(self.request.body).items():
      for ts, values in data.items():
        key = '%s/%s' % (dataset, ts)
        point = Sample(timestamp=datetime.datetime.strptime(ts, '%Y-%m-%d %H:%M:%S'),
                       dataset=dataset,
                       values=json.dumps(values))
        point.put()
        logging.info('Wrote %s' % point)


app = webapp2.WSGIApplication([
  ('/', MainPage),
  ('/api/add-data-point', AddDataPoint),
  ('/api/load-data', LoadData)
], debug=True)
