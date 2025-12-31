#!/usr/bin/env python3
# Simple HTTP server for serving Emscripten builds.
#
# Emscripten pthreads require SharedArrayBuffer, which needs these HTTP headers:
#   - Cross-Origin-Opener-Policy: same-origin
#   - Cross-Origin-Embedder-Policy: require-corp


import http.server
import os
from pathlib import Path


class HttpRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self) -> None:
        # Required headers for SharedArrayBuffer (needed for pthreads)
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        super().end_headers()


def main() -> None:
    directory = Path(__file__).parent / "build"
    port = 8000
    assert directory.is_dir()
    os.chdir(directory)
    http.server.ThreadingHTTPServer(
        ("127.0.0.1", port), HttpRequestHandler
    ).serve_forever()


if __name__ == "__main__":
    main()
