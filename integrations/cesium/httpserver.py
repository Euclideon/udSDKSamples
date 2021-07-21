from http.server import BaseHTTPRequestHandler,HTTPServer
from threading import Thread
import os

PORT = 8000

class ThreadedHTTPServer(HTTPServer):
    def process_request(self, request, client_address):
        thread = Thread(target=self.__new_request, args=(self.RequestHandlerClass, request, client_address, self))
        thread.start()

    def __new_request(self, handlerClass, request, address, server):
        handlerClass(request, address, server)
        self.shutdown_request(request)

class TileHandler(BaseHTTPRequestHandler):
  protocol_version = 'HTTP/1.1'

  def TH_SendHeaders(self, statusCode, endHeader=False, logRequest=False):
    if (statusCode == 200):
      self.send_response(statusCode)
    else:
      self.send_error(statusCode)
    
    self.send_header('Access-Control-Allow-Origin', "*")
    self.send_header('Access-Control-Allow-Headers', "Content-Length, Range, Content-Type, Accept, X-Requested-With")
    self.send_header('Access-Control-Allow-Methods', "HEAD, GET, OPTIONS")
    self.send_header('Cross-Origin-Opener-Policy', 'unsafe-none')
    # self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
    self.send_header('Cross-Origin-Resource-Policy', 'cross-origin')

    if (endHeader):
      self.end_headers()

    if (logRequest):
      lf = open('badreqs.log', 'a')
      lf.write(self.client_address[0] + " | " + self.path + "\n")
      lf.close()

  def do_GET(self, sendBody = True):
    filename = "./" + self.path

    print(filename)

    #see if it exists
    if not os.path.exists(filename):
      return self.TH_SendHeaders(404, endHeader=True, logRequest=True)
    else:
      fp = open(filename, 'rb');
      data = fp.read()
      fp.close()

      self.TH_SendHeaders(200)

      extensions_map = {
        'manifest': 'text/cache-manifest',
        'html': 'text/html',
        'png': 'image/png',
        'jpg': 'image/jpg',
        'svg':	'image/svg+xml',
        'css':	'text/css',
        'js':	'application/x-javascript',
        'json':	'application/json',
        'wasm': 'application/wasm'
      }

      if (filename.split(".")[-1] in extensions_map):
        self.send_header('Content-Type', extensions_map[filename.split(".")[-1]])

      self.send_header('Content-Length', str(len(data)))

      self.end_headers()

      if (sendBody):
        self.wfile.write(data)

  def do_POST(self):
    self.do_GET()
    
  def do_OPTIONS(self):
    self.do_GET(sendBody=False)

  def do_HEAD(self):
    self.do_GET(sendBody=False)
    
server = ThreadedHTTPServer(('', PORT), TileHandler)
print("Server running on port", PORT)
server.serve_forever()
