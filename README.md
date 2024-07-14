# ChatNet: Real-Time Messaging with Socket Programming and Multithreading

ChatNet is a multi-threaded chat application developed in C++ using socket programming and Winsock. It facilitates real-time messaging between clients and a central server, showcasing fundamental concepts in network communication and concurrent programming.

## Features

- **Socket Programming**: Utilizes Winsock API for socket creation, connection handling, and data exchange.
- **Multithreaded Design**: Implements threads for simultaneous message sending and receiving, ensuring smooth communication.
- **Client-Server Architecture**: Supports multiple clients connecting to a central server (localhost configuration).
- **Graceful Termination**: Allows clients to exit gracefully using the "quit" command, ensuring proper socket closure and resource cleanup.
- **User-Friendly Interface**: Simple command-line interface for entering chat names and messages.

## Getting Started

### Prerequisites

- Windows operating system (Winsock API used)
- C++ compiler (supporting C++11 or higher)
- Visual Studio

