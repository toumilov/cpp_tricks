# PostgreSQL proxy

This is a small proxy utility for capturing unencrypted SQL client requests in text file.

## Dependencies
- C++ compiler with C++ 17 support.
- Berkely sockets library.
- Pthread library
- GNU make

## Build
Makefile implements following built targets:
- **proxy** - build proxy application
- **clean** - clean project directory

## Usage
Proxy application needs few argument to start.

`<client port> <server IP> <server port(default = 5432)>`

1. **Client TCP port number** - where proxy is listening for incoming connections **(mandatory)**
2. **PostgreSQL server IPv4 address** - address where proxy will forward requests **(mandatory)**
3. **PostgreSQL server port** - server TCP port (default: 5432) **(optional)**

**Example:** `./proxy 6776 127.0.0.1`

### Output
Requests are logged to **log.txt** file.
File contents are truncated on every proxy start.

## Test utility
**test.py**

Needs **Python** and **psycopg2** package
```
sudo apt-get install libpq-dev
sudo pip install psycopg2
```

## Technical design
This proxy solution is capable of handling reasonably medium load. It implies, that requests have to be processed not in serial order.

### Threads
Proxy is designed as multithreaded application. Though, it doesn't spawn a thread for each client connection. Instead, it used a thread pool of fixed size.
1. **Listener thread (main)** - tracks incoming connections and creates a sessions.
2. **Worker thread** - tracks session list and performs basic scheduling.
3. **Thread pool** - fixed list of worker threads, which can perform asynchronous tasks.

### Errors
If protocol data is not following simple PostgreSQL message format, it may lead to connection drop (just like it's recommended in protocol documentation). Same for spuriously lost connection. Dangling and orphaned connections are automatically discarded.

### Verbose mode
By default proxy only pritns errors to the console. However, there is a way to enable debug logs (main.cpp):
```
Trace::instance().setup( Trace::Level::Error ); // <= change "Error" to "Debug"
```

### Further improvements
1. Session list management - client prioritization
2. Session scheduling - a better algorithm (now it's simple "round-robin")
3. Use epoll for descriptor state handleing - now it's *select*
