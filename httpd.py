
import BaseHTTPServer
import SimpleHTTPServer
import SocketServer
import logging
import os
import sys


logging.getLogger().setLevel(logging.INFO)


# Using 'localhost' means that we only accept connections
# via the loop back interface.
SERVER_PORT = 5103
SERVER_HOST = 'localhost'


# the sole purpose of this class is to make the BaseHTTPServer threaded
class ThreadedServer(SocketServer.ThreadingMixIn,
                     BaseHTTPServer.HTTPServer):
  pass


def Run(server_address,
        server_class=ThreadedServer,
        handler_class=SimpleHTTPServer.SimpleHTTPRequestHandler):
  httpd = server_class(server_address, handler_class)
  logging.info('started server on port %d', httpd.server_address[1])
  httpd.serve_forever()


if __name__ == '__main__':
  os.chdir('out')
  if len(sys.argv) > 1:
    Run((SERVER_HOST, int(sys.argv[1])))
  else:
    Run((SERVER_HOST, SERVER_PORT))
