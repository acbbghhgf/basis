#! /usr/bin/env python
# -*- coding:utf-8 -*-

import multiprocessing
import optparse
import httplib
import string
import json
import sys
import os

scriptTemplate = ''' local inst = new([=[$RES]=])
    inst:calc([=[$PARAM]=])
    local result = inst:getResult()
    reply(200, result)
    inst:close()
'''

def read_test_file(fn):
    with open(fn, 'r') as f:
        for line in f.readlines():
            line = line.strip()
            if not len(line):
                continue
            if line.startswith('#'):
                continue
            tmp = line.split('\t')
            yield {'res':tmp[0],'input':tmp[1]}


def test_item(item, conn):
    param = {
        'input' : item['input'],
        'clientId' : str(os.getpid()),
    }
    param = json.dumps(param)
    t = string.Template(scriptTemplate)
    script = t.substitute({'RES':item['res'],'PARAM':param})
    conn.request('GET', '/', script)
    resp = conn.getresponse()
    result = resp.read()
    print(result)


def test_route(opt):
    conn = httplib.HTTPConnection(opt.ip + ':' + str(opt.port))
    for i in range(opt.round):
        for item in read_test_file(opt.fn):
            test_item(item, conn)
    conn.close()


def run(opt):
    procs = []
    for i in range(opt.nproc):
        p = multiprocessing.Process(target=test_route, args=[opt])
        p.start()
        procs.append(p)

    for p in procs:
        p.join()


if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-i', '--ip', dest='ip',
                      type='string', default='127.0.0.1')
    parser.add_option('-p', '--port', dest='port', type='int', default=9999)
    parser.add_option('-r', '--round', dest='round', type='int', default=1)
    parser.add_option('-n', '--nproc', dest='nproc', type='int', default=1)
    parser.add_option('-f', '--fn', dest='fn', type='string', default='test.t')
    opt, args = parser.parse_args()
    run(opt)
