#!/bin/python
#encoding: utf-8

"""
qtk_httplib is simplified httplib, exclude chunked code
response is not necessary for request
"""

from array import array
import os
import socket
import mimetools
try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO


HTTP_PORT = 80

_UNKNOWN = 'UNKNOWN'

_CS_IDLE = 'Idle'
_CS_REQ_STARTED = 'Request-started'
_CS_REQ_SENT = 'Request-sent'

OK = 200

# maximal amount of data to read at one time in _safe_read
MAXAMOUNT = 1048576

# maximal line length when calling readline().
_MAXLINE = 65536

# maximum amount of headers accepted
_MAXHEADERS = 100


class HTTPException(Exception):
    pass


class InvalidURL(HTTPException):
    pass


class UnknownProtocol(HTTPException):
    def __init__(self, version):
        self.args = version,
        self.version = version


class IncompleteRead(HTTPException):
    def __init__(self, partial, expected=None):
        self.args = partial,
        self.partial = partial
        self.expected = expected

    def __repr__(self):
        if self.expected is not None:
            e = ', %i more expected' % self.expected
        else:
            e = ''
        return 'Incompleteread(%i bytes read%s)' % (len(self.partial), e)

    def __str__(self):
        return repr(self)


class ImproperConnectionState(HTTPException):
    pass


class CannotSendRequest(ImproperConnectionState):
    pass


class CannotSendHeader(ImproperConnectionState):
    pass


class ResponseNotReady(ImproperConnectionState):
    pass


class BadStatusLine(HTTPException):
    def __init__(self, line):
        if not line:
            line = repr(line)
        self.args = line,
        self.line = line


class LineTooLong(HTTPException):
    def __init__(self, line_type):
        HTTPException.__init__(self, "got more than %d bytes when reading %s"
                               % (MAX_LINE, line_type))


class HTTPMessage(mimetools.Message):

    def addheader(self, key, value):
        """Add header for field key handling repeats."""
        prev = self.dict.get(key)
        if prev is None:
            self.dict[key] = value
        else:
            combined = ', '.join((prev, value))
            self.dict[key] = combined

    def addcontinue(self, key, more):
        """Add more field data from a continuation line."""
        prev = self.dict[key]
        self.dict[key] = prev + '\n' + more

    def readheaders(self):
        """Read header lines.

        Read header lines up to the entirely blank line that terminates them.
        The (normally blank) line that ends the headers is skipped, but not
        included in the returned list.  If a non-header line ends the headers,
        (which is an error), an attempt is made to backspace over it; it is
        never included in the returned list.

        The variable self.status is set to the empty string if all went well,
        otherwise it is an error message.  The variable self.headers is a
        completely uninterpreted list of lines contained in the header (so
        printing them will reproduce the header exactly as it appears in the
        file).

        If multiple header fields with the same name occur, they are combined
        according to the rules in RFC 2616 sec 4.2:

        Appending each subsequent field-value to the first, each separated
        by a comma. The order in which header fields with the same field-name
        are received is significant to the interpretation of the combined
        field value.
        """
        self.dict = {}
        self.unixfrom = ''
        self.headers = hlist = []
        self.status = ''
        headerseen = ''
        firstline = 1
        startofline = unread = tell = None
        if hasattr(self.fp, 'unread'):
            unread = self.fp.unread
        elif self.seekable:
            tell = self.fp.tell
        while True:
            if len(hlist) > _MAXHEADERS:
                raise HTTPException('get more than %d headers' % _MAXHEADERS)
            if tell:
                try:
                    startofline = tell()
                except IOError:
                    startofline = tell = None
                    self.seekable = 0
            line = self.fp.readline(_MAXLINE + 1)
            if len(line) > _MAXLINE:
                raise LineTooLong('header line')
            if not line:
                self.status = 'EOF in headers'
                break

            if firstline and line.startswith('From '):
                self.unixfrom = self.unixfrom + line
                continue
            firstline = 0
            if headerseen and line[0] in ' \t':
                hlist.append(line)
                self.addcontinue(headerseen, line.strip())
                continue
            elif self.iscomment(line):
                continue
            elif self.islast(line):
                break
            headerseen = self.isheader(line)
            if headerseen:
                hlist.append(line)
                self.addheader(headerseen, line[len(headerseen)+1:].strip())
                continue
            else:
                if not self.dict:
                    self.status = 'No headers'
                else:
                    self.status = 'Non-header line where header expected'
                if unread:
                    unread(line)
                elif tell:
                    self.fp.seek(startofline)
                else:
                    self.status = self.status + '; bad seek'
                break;


class HTTPResponse:
    def __init__(self, sock, debuglevel=0, method=None, buffering=False):
        if buffering:
            self.fp = sock.makefile('rb')
        else:
            self.fp = sock.makefile('rb', 0)
        self.debuglevel = debuglevel
        self._method = method

        self.msg = None

        self.version = _UNKNOWN
        self.status = _UNKNOWN
        self.reason = _UNKNOWN
        self.length = _UNKNOWN
        self.will_close = _UNKNOWN

    def _read_status(self):
        line = self.fp.readline(_MAXLINE + 1)
        if len(line) > _MAXLINE:
            raise LineTooLong("header line")
        if self.debuglevel > 0:
            print "reply:", repr(line)
        if not line:
            raise BadStatusLine(line)
        try:
            [version, status, reason] = line.split(None, 2)
        except ValueError:
            try:
                [version, status] = line.split(None, 1)
                reason = ""
            except ValueError:
                version = ""
        if not version.startswith("HTTP/"):
            self.close()
            raise BadStatusLine(line)

        try:
            status = int(status)
            if status < 100 or status > 999:
                raise BadStatusLine(line)
        except ValueError:
            raise BadStatusLine(line)
        return version, status, reason

    def begin(self):
        if self.msg is not None:
            return

        version, status, reason = self._read_status()
        self.status = status
        self.reason = reason.strip()
        if version == 'HTTP/1.0':
            self.version = 10
        elif version.startswith('HTTP/1.'):
            self.version = 11
        elif version == 'HTTP/0.9':
            self.version = 9
        else:
            raise UnknownProtocol(version)

        self.msg = HTTPMessage(self.fp, 0)
        if self.debuglevel > 0:
            for hdr in self.msg.headers:
                print "header:", hdr,

        self.msg.fp = None

        self.will_close = self._check_close()
        length = self.msg.getheader('content-length')
        if length:
            try:
                self.length = int(length)
            except ValueError:
                self.length = None
            else:
                if self.length < 0:
                    self.length = None
        else:
            self.length = None

    def _check_close(self):
        conn = self.msg.getheader('connection')
        if self.version == 11:
            conn = self.msg.getheader('connection')
            if conn and 'close' in conn.lower():
                return True
            return False

        if self.msg.getheader('keep-alive'):
            return False

        if conn and 'keep-alive' in conn.lower():
            return False

        return True

    def close(self):
        if self.fp:
            self.fp.close()
            self.fp = None

    def isclosed(self):
        return self.fp is None

    def read(self):
        if self.fp is None:
            return ''

        if self._method == 'HEAD':
            self.close()
            return ''

        if self.length is None:
            s = self.fp.read()
        else:
            try:
                s = self._safe_read(self.length)
            except IncompleteRead:
                self.close()
                raise
            self.length = 0
        self.close()
        return s

    def _safe_read(self, amt):
        s = []
        while amt > 0:
            chunk = self.fp.read(min(amt, MAXAMOUNT))
            if not chunk:
                raise IncompleteRead(''.join(s), amt)
            s.append(chunk)
            amt -= len(chunk)
        return ''.join(s)

    def fileno(self):
        return self.fp.fileno()

    def getheader(self, name, default=None):
        if self.msg is None:
            raise ResponseNotReady()
        return self.msg.getheader(name, default)

    def getheaders(self):
        """Return list of (header, value) tuples."""
        if self.msg is None:
            raise ResponseNotReady()
        return self.msg.items()


class HTTPRequest:
    def __init__(self, sock, debuglevel=0, buffering=False):
        if buffering:
            self.fp = sock.makefile('rb')
        else:
            self.fp = sock.makefile('rb', 0)
        self.debuglevel = debuglevel

        self.msg = None

        self.version = _UNKNOWN
        self.method = _UNKNOWN
        self.url = _UNKNOWN
        self.length = _UNKNOWN
        self.will_close = _UNKNOWN

    def _read_method(self):
        line = self.fp.readline(_MAXLINE + 1)
        if len(line) > _MAXLINE:
            raise LineTooLong("header line")
        if self.debuglevel > 0:
            print "reply:", repr(line)
        if not line:
            raise BadStatusLine(line)
        try:
            [method, url, version] = line.split(None, 2)
        except ValueError:
            try:
                [version, status] = line.split(None, 1)
                reason = ""
            except ValueError:
                version = ""
        if not version.startswith("HTTP/"):
            self.close()
            raise BadStatusLine(line)

        if method not in ('PUT', 'POST', 'GET', 'HEAD', 'DELETE'):
            self.close()
            raise BadStatusLine(line)

        return method, url, version

    def begin(self):
        if self.msg is not None:
            return

        method, url, version = self._read_method()
        self.method = method
        self.url = url
        if version == 'HTTP/1.0':
            self.version = 10
        elif version.startswith('HTTP/1.'):
            self.version = 11
        elif version == 'HTTP/0.9':
            self.version = 9
        else:
            raise UnknownProtocol(version)

        self.msg = HTTPMessage(self.fp, 0)
        if self.debuglevel > 0:
            for hdr in self.msg.headers:
                print "header:", hdr,

        self.msg.fp = None

        self.will_close = self._check_close()
        length = self.msg.getheader('content-length')
        if length:
            try:
                self.length = int(length)
            except ValueError:
                self.length = None
            else:
                if self.length < 0:
                    self.length = None
        else:
            self.length = 0

    def _check_close(self):
        conn = self.msg.getheader('connection')
        if self.version == 11:
            conn = self.msg.getheader('connection')
            if conn and 'close' in conn.lower():
                return True
            return False

        if self.msg.getheader('keep-alive'):
            return False

        if conn and 'keep-alive' in conn.lower():
            return False

        return True

    def close(self):
        if self.fp:
            self.fp.close()
            self.fp = None

    def isclosed(self):
        return self.fp is None

    def read(self):
        if self.fp is None:
            return ''
        if self.length is None:
            s = self.fp.read()
        else:
            try:
                s = self._safe_read(self.length)
            except IncompleteRead:
                self.close()
                raise
            self.length = 0
        self.close()
        return s

    def _safe_read(self, amt):
        s = []
        while amt > 0:
            chunk = self.fp.read(min(amt, MAXAMOUNT))
            if not chunk:
                raise IncompleteRead(''.join(s), amt)
            s.append(chunk)
            amt -= len(chunk)
        return ''.join(s)

    def fileno(self):
        return self.fp.fileno()

    def getheader(self, name, default=None):
        if self.msg is None:
            raise ResponseNotReady()
        return self.msg.getheader(name, default)

    def getheaders(self):
        """Return list of (header, value) tuples."""
        if self.msg is None:
            raise ResponseNotReady()
        return self.msg.items()


class HTTPConnection:
    _http_vsn_str = 'HTTP/1.1'
    response_class = HTTPResponse
    request_class = HTTPRequest
    defaultport = HTTP_PORT
    debuglevel = 0

    def __init__(self, host, port=None, timeout=socket._GLOBAL_DEFAULT_TIMEOUT):
        self.timeout = timeout
        self.sock = None;
        self._buffer = []
        self.__response = None
        self.__request = None
        self.__state = _CS_IDLE
        self.method = None

        self._set_hostport(host, port)

    def _set_hostport(self, host, port):
        if port is None:
            i = host.rfind(':')
            j = host.rfind(']')
            if i > j:
                try:
                    port = int(host[i+1:])
                except ValueError:
                    if host[i+1:] == '':
                        port = self.default_port
                    else:
                        raise InvalidURL('nonnumeric port: "%s"' % host[i+1:])
                host = host[:i]
            else:
                port = self.default_port
            if host and host[0] == '[' and host[-1] == ']':
                hsot = host[1:-1]
        self.host = host
        self.port = port

    def set_debuglevel(self, level):
        self.debuglevel = level

    def connect(self):
        """Connect to host and port specified in __init__."""
        self.sock = socket.create_connection((self.host, self.port),
                                             self.timeout)

    def close(self):
        """Close the connection to the HTTP server."""
        if self.sock:
            self.sock.close()
            self.sock = None
        if self.__response:
            self.__response.close()
            self.__response = None
        if self.__request:
            self.__request.close()
            self.__request = None
        self.__state = _CS_IDLE

    def send(self, data):
        """Send data to the server."""
        if self.sock is None:
            self.connect()

        if self.debuglevel > 0:
            print "send:", repr(data)
        blocksize = 8192
        if hasattr(data, 'read') and not isinstance(data, array):
            if self.debuglevel > 0:
                print "sending a readable"
            datablock = data.read(blocksize)
            while datablock:
                self.sock.sendall(datablock)
                datablock = data.read(blocksize)
        else:
            self.sock.sendall(data)

    def _output(self, s):
        """Add a line of output to the current request buffer.
        Assumes that the line does *not* end with \\r\\n.
        """
        self._buffer.append(s)

    def _send_output(self, message_body=None):
        """Send the currently buffered request and clear the buffer.
        Appends an extra \\r\\n to buffer.
        A message_body may be specified, to be appended to the request.
        """
        self._buffer.extend(("", ""))
        msg = "\r\n".join(self._buffer)
        del self._buffer[:]
        if isinstance(message_body, str):
            msg += message_body
            message_body = None
        self.send(msg)
        if message_body is not None:
            self.send(message_body)

    def putrequest(self, method, url):
        """Send a request to the server.

        `method' specifies an HTTP request method, e.g. 'GET'.
        `url' specifies the object being requested, e.g. '/index.html'.
        """

        if self.__state == _CS_REQ_STARTED:
            raise CannotSendRequest()
        else:
            self.__state = _CS_REQ_STARTED

        self._method = method
        if not url:
            url = '/'
        hdr = '%s %s %s' % (method, url, self._http_vsn_str)

        self._output(hdr)

    def putheader(self, header, *values):
        """Send a request headerline to the server.
        For example: h.putheader('Accept', 'text/html')
        """
        if self.__state != _CS_REQ_STARTED:
            print self.__state
            raise CannotSendHeader()

        hdr = '%s: %s' % (header, '\r\n\t'.join([str(v) for v in values]))
        self._output(hdr)

    def endheaders(self, message_body=None):
        """Indicate that last header line has been sent to the server.

        This method sends the request to the server. The optional
        message_body argument can be used to pass a message body
        associated with the request.  The message body will be sent in
        the same package as message headers if it is string, otherwise it is
        sent as a separate packet.
        """
        if self.__state == _CS_REQ_STARTED:
            self.__state = _CS_REQ_SENT
        else:
            raise CannotSendHeader()
        self._send_output(message_body)

    def request(self, method, url, body=None, headers={}):
        """Send a complete request to the server."""
        self._send_request(method, url, body, headers)

    def _set_content_length(self, body):
        # Set the content-length based on the body.
        thelen = None
        try:
            thelen = str(len(body))
        except TypeError, te:
            try:
                thelen = str(os.fstat(body.fileno()).st_size)
            except (AttributeError, OSError):
                if self.debuglevel > 0:
                    print "Cannot stat!!"

        if thelen is not None:
            self.putheader('Content-Length', thelen)

    def _send_request(self, method, url, body, headers):
        header_names = dict.fromkeys([k.lower() for k in headers])

        self.putrequest(method, url)

        if body is not None and 'content-length' not in header_names:
            self._set_content_length(body)
        for hdr, value in headers.iteritems():
            self.putheader(hdr, value)
        self.endheaders(body)

    def getresponse(self, buffering=False):
        "Get the response from the server."

        if self.__response and self.__response.isclosed():
            self.__response = None

        args = (self.sock,)
        kwds = {'method': self._method}
        if self.debuglevel > 0:
            args += (self.debuglevel,)
        if buffering:
            kwds['buffering'] = True
        response = self.response_class(*args, **kwds)

        response.begin()
        assert response.will_close != _UNKNOWN
        self.__state = _CS_IDLE

        if response.will_close:
            self.close()
        else:
            self.__response = response

        return response

    def getrequest(self, buffering=False):
        "Get the response from the server."

        if self.__request and self.__request.isclosed():
            self.__request = None

        args = (self.sock,)
        kwds = {}
        if self.debuglevel > 0:
            args += (self.debuglevel,)
        if buffering:
            kwds['buffering'] = True
        request = self.request_class(*args, **kwds)

        request.begin()
        assert request.will_close != _UNKNOWN
        self.__state = _CS_IDLE

        if request.will_close:
            self.close()
        else:
            self.__request = request

        return request
