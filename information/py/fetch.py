import qtk_httplib as httplib
import json
import sys


def fetch(opt):
    topic = opt.topic.replace('_', '/')
    con = httplib.HTTPConnection(opt.ip, opt.port)
    data = {}
    data[topic] = opt.checkpoint
    con.request("POST", "/_watch", json.dumps(data))

    if opt.output:
        fp = open(opt.output, "w")
    else:
        fp = sys.stdout
    while True:
        reply = con.getrequest()
        data = reply.read()
        if len(data):
            fp.write(data)
            fp.write("\n")
            fp.flush()
        else:
            break
    fp.close()


if __name__ == '__main__':
    import optparse
    parser = optparse.OptionParser()
    parser.add_option('-i', '--ip', dest='ip', type='string', default='127.0.0.1')
    parser.add_option('-p', '--port', dest='port', type='int', default=10000)
    parser.add_option('-o', '--output', dest='output', type='string')
    parser.add_option('-t', '--topic', dest='topic', type='string')
    parser.add_option('-c', '--checkpoint', dest='checkpoint', type='string', default='')
    parser.add_option('-s', '--stream', dest='stream', type='int', default=0)
    (opt, args) = parser.parse_args()
    fetch(opt)
