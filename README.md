# Tiny HTTP/HTTPS Server

A simple HTTP/HTTPS server written in C using OpenSSL for educational purposes. Supports both secure (HTTPS) and plain (HTTP) modes.

## Running Modes

### HTTPS Mode (Default)
Secure server with SSL/TLS encryption - requires certificates.

### HTTP Mode (--no-tls flag)
Plain HTTP server without encryption - no certificates needed.

## Setup Instructions

### Option 1: Quick Start (HTTP Mode)

```bash
# Build the server
make

# Run as plain HTTP server (no certificates needed)
./.bin/tinyserver --no-tls
```

Test at: `http://localhost:8080`

### Option 2: Secure HTTPS Mode

**⚠️ IMPORTANT: Requires SSL certificates (NOT included for security reasons)**

#### 1. Generate SSL Certificates

```bash
# Generate private key (KEEP THIS SECRET!)
openssl genrsa -out server.key 2048

# Generate certificate signing request
openssl req -new -key server.key -out server.csr

# Generate self-signed certificate (for testing only)
openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt

# Generate CA certificate (for client verification)
openssl req -new -x509 -days 365 -key server.key -out ca.crt
```

#### 2. Run HTTPS Server

```bash
# Build the server
make

# Run as HTTPS server (default mode)
./.bin/tinyserver
```

Test at: `https://localhost:8080`

**Note:** You'll get a security warning because it's a self-signed certificate. This is normal for development.

## Command Line Options

- **No arguments**: Run as HTTPS server (requires certificates)
- **--no-tls**: Run as plain HTTP server (no certificates needed)

## File Structure

```
TinyServer/
├── src/
│   └── tinyserver.c    # Main server code
├── .bin/               # Compiled executables (auto-generated)
├── .obj/               # Object files (auto-generated)
├── makefile           # Build configuration
├── .gitignore         # Git ignore rules
└── README.md          # This file

# These files are created by you (NOT in git):
├── server.key         # Private key (NEVER commit!)
├── server.crt         # Server certificate
└── ca.crt            # CA certificate
```

## Security Best Practices

- **Never commit private keys** (`.key` files) to version control
- **Never commit certificates** with real private keys to public repositories
- Use proper certificate authorities for production
- This is for educational purposes only - not production ready

## Learning Objectives

This project demonstrates:
- C socket programming
- SSL/TLS encryption with OpenSSL
- HTTP protocol handling
- Certificate-based authentication
- Proper build system with Makefiles
- Command line argument parsing