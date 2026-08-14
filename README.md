# Distributive KV Store

A Redis-inspired in-memory key-value store built from scratch in C++.

## Current Features

- TCP client-server communication
- Custom text-based protocol
- PING / PONG
- SET key value
- GET key
- In-memory storage using `unordered_map`
- Persistent TCP connection for multiple commands

## Tech Stack

- C++
- TCP
- Windows Winsock
- Git / GitHub

## Example

SET name Piyush
OK

GET name
Piyush
