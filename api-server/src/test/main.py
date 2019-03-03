#! /usr/bin/env python3
# -*- coding:utf-8 -*-
from req.req_pb2 import payload as req
import req.req_pb2 as req_pb2
from resp.resp_pb2 import resp
from utils import sha1rsa
from config import config
import multiprocessing
import websocket
import json
import uuid
import requests
import time
import os
import sys
import urllib
import hashlib
import base64 

ACT_START = req_pb2._ACT_ENUM.values_by_name['START'].number
ACT_APPEND = req_pb2._ACT_ENUM.values_by_name['APPEND'].number
ACT_END = req_pb2._ACT_ENUM.values_by_name['END'].number

#ret = {}

mime_dict = {
    'wav': '"audio/x-wav"',
    'ogg': '"ogg"',
}

def get_auth_info(web_url):
    secretKey = '0063909a-ef58-11e6-8a33-00e04c12c2c7'
    appId = 'f4b7c86a-ef57-11e6-8aa7-00e04c12c2c7'
    deviceId = str(os.getpid())
    data_url = {}
    data_url['appId'] = appId
    data_url['deviceId'] = deviceId
    data_url['time'] = str(int(time.time()))
    sign_str = appId + deviceId + data_url['time'] + secretKey
    client_sign = hashlib.sha1(sign_str.encode('utf-8')).hexdigest()
    # Ｃ端标识加密后发送
    data_url['sign'] = sha1rsa.encrypt(client_sign, config.RSA_PUBLIC_KEY, config.RSA_PASSPHRASE)
    return data_url

def post_web(web_url):
    send_json = get_auth_info(web_url)
    print('send_jsondata=====', send_json)
    header = {
        'Content-Type':'application/x-www-form-urlencoded',    
    }
    r = requests.post(url=web_url, headers=header, data=send_json)
    js = r.json()
    print('js========',js)
    #if js['code'] == 0:
    fdst = r.headers['Authorization']
    print('Authorization====',fdst)
    return fdst


def read_audio(fn):
    with open(fn, 'rb') as f:
        while True:
            data = f.read(4096)
            if not data:
                break
            yield data
            time.sleep(len(data)/32000.0)


class client(object):
    def __init__(self, server, token):
        self.url = "ws://" + server
        self.ws = websocket.WebSocket()
        self.ws.connect(self.url, subprotocols=['default'],
			header = ['AUTHORIZATION: ' + token])

    def __del__(self):
        pass

    def __set_start_req(self, args):
        request = req()
        request.act = ACT_START
        request.obj = args['coreType']

        # Too Ugly
        text = args.get('text')
        if text != None:
            request.desc.param.text = text

        res = args.get('res')
        if res != None:
            request.desc.param.res = res

        useStream = args.get('useStream')
        if useStream != None:
            request.desc.param.useStream = useStream

        volume = args.get('volume')
        if volume != None:
            request.desc.param.volume = volume

        speed = args.get('speed')
        if speed != None:
            request.desc.param.speed

        pitch = args.get('pitch')
        if pitch != None:
            request.desc.param.pitch = pitch

        env = args.get('env')
        if env != None:
            request.desc.param.env = env

        semRes = args.get('semRes')
        if semRes != None:
            request.desc.param.semRes = semRes

        ebnfRes = args.get('ebnfRes')
        if ebnfRes != None:
            request.desc.param.ebnfRes = ebnfRes

        iType = args.get('iType')
        if iType != None:
            request.desc.param.iType = iType

        request.extra.hook = str(uuid.uuid4())
        request.extra.appId = 'qdreamer_test'
        request.extra.deviceId = str(os.getpid())
        if self.audio:
            request.desc.audio.audioType = mime_dict[self.audio.split('.')[-1]]
            request.desc.audio.sampleRate = 16000
            request.desc.audio.sampleBytes = 2
            request.desc.audio.channel = 1
        self.request = request

    def __snd_start(self):
        self.ws.send(self.request.SerializeToString())
        t1 = time.time()
        return t1

    def __snd_data(self, data):
        request = req()
        request.act = ACT_APPEND
        request.obj = 'audio'
        request.data = data
        request.extra.hook = self.request.extra.hook
        self.ws.send(request.SerializeToString())

    def __snd_stop(self):
        request = req()
        request.act = ACT_END
        request.extra.hook = self.request.extra.hook
        self.ws.send(request.SerializeToString())

    def __get_resp(self):
        response = resp()
        response.ParseFromString(self.ws.recv())
        return response

    def do(self, param, ret_q, log): #add time t1 t2 ['delay]
        ret = {}
        print("xxxxxxxxxx")
        items = param.split('\t')
        audio = items[0]
        self.audio = (None if audio=='None' else audio)
        args = eval('{' + items[1] + '}')
        self.__set_start_req(args)
        t1 = self.__snd_start()
        if self.audio:
            for data in read_audio(audio):
                self.__snd_data(data)
            self.__snd_stop()
        append = False
        while True:
            response = self.__get_resp()
            if response.dlg:
                print('======= +> dlg', response.dlg)
            if response.type == 'text/plain':
                print('=======> type = text/plain', response.bin.decode())
            elif response.type == 'audio/mpeg':
                flag = ('ab' if append else 'wb')
                with open('test.mp3', flag) as f:
                    append = True
                    print('write audio ' , str(len(response.bin)))
                    f.write(response.bin)
            elif response.type == 'ERROR':
                print('Got Error ' , response.bin.decode())
            if response.isEnd:
                break
        t2 = time.time()
        ret['delay'] = t2 - t1
        log.write('delay \t'+ str(ret['delay']) + '\t\n')
        log.flush()



def test_route(opt, ret_q, log):
    web_url = 'http://'+opt.webIP+ '/api/v1/auth/'
    token = post_web(web_url)
    c = client(opt.target, token)
    for line in open(opt.fn):
        # print("======>>>>>", line)
        if not len(line) or line.startswith('#'):
            print("no client")
            continue
        c.do(line, ret_q, log)

def run(opt):
    #timelist = multiprocessing.Manager().list()
    procs = []
    ret_q = multiprocessing.Manager().Queue()
    log = open("httpc.log", 'w')
    for i in range(1, opt.n):
        for i in range(i): #opt.n
            print("xxx1111")
            p = multiprocessing.Process(target=test_route, args=(opt, ret_q, log))
            p.start()
            print("xxx22222")        
            procs.append(p)

        for p in procs:
            try:                
                print('waitpid:', p.pid)
                os.waitpid(p.pid, 0)
            except OSError as e:
                if e.errno != 10:                 
                    print(sys.exc_info())
            except:
                print(sys.exc_info())
    
    tot = 0
    delay = 0
    with open("httpc.log") as f:
        for line in f:
            tot += 1
            if not len(line) or line.startswith('#'):
                continue
            log_text = line.split('\t')
            delay += float(log_text[1])
        
    print('\n\n')
    print('#' * 20, 'tot result', '#' * 20)
    print('tot', tot, 'delay', delay)
    print('delay', delay - tot if tot else 0, 's')
    print('#' * 50)
    with open("time.log", "w") as fp:
        fp.write(str(delay - tot) + '\n')
        fp.close()
        


if __name__ == '__main__':
    import optparse
    parser = optparse.OptionParser()
    parser.add_option("-t", "--target", dest="target", type="string", default='127.0.0.1:18761')
    parser.add_option("-n", "--nproc", dest="n", type="int", default=2)
    parser.add_option("-f", "--fn", dest="fn", type="string", default='t.t')
    parser.add_option("-p", "--webIP", dest="webIP", type="string", default='192.168.0.177:18888')
    (opt, args) = parser.parse_args()
    #websocket.enableTrace(True)
    run(opt)
