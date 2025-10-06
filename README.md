⚙️ Project Overview

This project implements an IRC server that allows multiple clients to connect, join channels, and communicate — just like on a real IRC network.

It’s compatible with common IRC clients such as irssi, weechat, or even netcat for testing.

The server handles:

  -Multiple clients simultaneously

  -Channels and modes

  -Authentication and registration

  -Flooding and ghost/suspended clients gracefully

  -Full RFC-compliant message formatting

  🧩 Implemented Features

✅ Core commands
PASS, NICK, USER, JOIN, PART, PRIVMSG, QUIT, PING/PONG

✅ Channel management
TOPIC, KICK, INVITE, MODE

✅ Channel modes
Supports +n (no external messages) and +t (topic settable only by ops)

✅ Operator privileges
Automatic operator on channel creation, proper operator handling on rejoin

✅ Robust server loop
Non-blocking sockets with poll() and graceful cleanup of disconnected clients

✅ Flood resistance
Suspended clients are detected, and the server stays alive and responsive

🚀 How to Run (bash)
  1️⃣ Compile
    make
  2️⃣ Run the server
    ./ircserv <port> <password>

🧠 Technical Highlights

  -Language: C++17

  -Socket type: IPv6 (also supports IPv4-mapped clients)

  -I/O model: Non-blocking with poll()

  -Error handling: Graceful disconnect & cleanup

  -Memory: Leak-free under stress conditions

  -RFC compliance: Based on RFC 1459 / 2812

👥 Authors

@Emileiniel2013 — Command parsing, channel logic, protocol compliance

@silndoj  — Server architecture, poll system, connection management
  
