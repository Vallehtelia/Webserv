import http.client

conn = http.client.HTTPConnection("localhost", 8002)

# Send a chunked request
# conn.request("POST", "/upload", body=None, headers={"Transfer-Encoding": "chunked"})
# conn.send(b"5\r\nHello\r\n")
# conn.send(b"6\r\nWorld!\r\n")
# conn.send(b"0\r\n\r\n")
conn.request("GET", "/", body=None, headers={"Transfer-Encoding": "chunked"})
conn.send(b"5\r\nHello\r\n")
conn.send(b"6\r\nWorld!\r\n")
conn.send(b"0\r\n\r\n")
response = conn.getresponse()
print(response.status, response.reason)
