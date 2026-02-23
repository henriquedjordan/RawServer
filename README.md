# RawServer
An HTTP server with no frameworks, no libraries, no excuses

## Features
-  Accepts real TCP connections
-  Manually parses HTTP/1.1 requests (yes, with `\r\n`)
-  Crafts valid HTTP responses with your bare hands

---

## How to use

```bash
make

```

```bash
./server

```

Then 
```bash
curl http://localhost:8080
