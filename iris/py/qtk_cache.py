import requests
import uuid
import multiprocessing
import sys
import os
import qtk_httplib as httplib
import time
import requests
import json
import random
appid = '123456789'


class qtk_cachec():
    def __init__(self, ip, port, path='/'):
        self.ip = ip
        self.port = port
        self._con = httplib.HTTPConnection(ip, port)
        self._path = path

    def close(self):
        self._con.close()

    def request(self, method, uri, data=None, headers=None):
        if headers is None:
            headers = {}
        if uri[0] != '/':
            uri = self._path + '/' + uri
        self._con.request(method, uri, data, headers)

    def get_response(self):
        reply = self._con.getresponse()
        headers = reply.getheaders()
        data = reply.read()
        if reply.status >= 400:
            print 'ERROR'
            print headers
            headers = {k: v for (k, v) in headers}
            # sys.exit()
            return None, None
        return data

    def get_request(self):
        reply = self._con.getrequest()
        headers = reply.getheaders()
        data = reply.read()
        return reply.url, data


def test_produce(con, topic, stream):
    for x in "abc":
        con.request('PUT', topic, x*1024)
    if stream:
        con.request('PUT', topic, "")

def test_consume(con, stream):
    for x in "abc":
        print '-'
        con.get_request()
        print x
    if stream:
        print "*******"
        con.get_request()
        print "======="

def test_watch(con, topic):
    data = {}
    data[topic] = ''
    con.request('POST', '/_watch', json.dumps(data))

def test_unwatch(con, topic):
    data = [
        topic,
    ]
    con.request('POST', '/_unwatch', json.dumps(data))

def test_topic(opt):
    con_p = qtk_cachec(opt.ip, opt.port, '/')
    for i in xrange(2, 10000000):
        topic = '/%d/%d' % (os.getpid(), i)
        con_w = qtk_cachec(opt.ip, opt.port, '/')
        stream = random.random() < 0.5
        test_watch(con_w, topic)
        test_produce(con_p, topic, stream)
        test_consume(con_w, stream)
        test_unwatch(con_w, topic)
        con_w.close()


def test_route(opt):
    for k in xrange(opt.round):
        test_topic(opt)

def test(opt):
    procs = []
    for i in range(opt.n):
        p = multiprocessing.Process(target=test_route, args=[opt])
        p.start()
        procs.append(p)

    for p in procs:
        try:
            os.waitpid(p.pid, 0)
        except OSError, e:
            if e.errno != 10:
                print sys.exc_info()
        except:
            print sys.exc_info()

if __name__ == '__main__':
    import optparse
    parser = optparse.OptionParser()
    parser.add_option('-i', '--ip', dest='ip', type='string', default='127.0.0.1')
    parser.add_option('-p', '--port', dest='port', type='int', default=10000)
    parser.add_option('-n', '--nproc', dest='n', type='int', default=1)
    parser.add_option('-r', '--round', dest='round', type='int', default=1)
    (opt, args) = parser.parse_args()
    test(opt)
