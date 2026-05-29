#!/usr/bin/env python3
"""Static server for the Lanarts emscripten content lab.

The build uses pthreads (SharedArrayBuffer), which the browser only enables for
cross-origin-isolated pages, so we must send COOP/COEP headers that the stock
http.server does not. Serves an emscripten build dir (default the debug build).

Usage: python3 web/content_lab/serve.py [port] [build_dir]
"""
import sys, http.server, socketserver

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8765
DIRECTORY = sys.argv[2] if len(sys.argv) > 2 else "build_debug_emscripten/src"


class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cache-Control", "no-store")
        super().end_headers()


class Server(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True


print(f"Lanarts content lab: http://localhost:{PORT}/lanarts.html  (dir={DIRECTORY})")
with Server(("127.0.0.1", PORT), Handler) as httpd:
    httpd.serve_forever()
