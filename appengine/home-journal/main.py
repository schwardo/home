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
  ('/api/add-data-point', AddDataPoint)
], debug=True)
