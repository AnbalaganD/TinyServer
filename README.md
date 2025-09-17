# Tiny SSL Server

A simple HTTPS server written in C using OpenSSL for educational purposes.

## Security Notice

**⚠️ IMPORTANT: This project requires SSL certificates that are NOT included in this repository for security reasons.**

## Setup Instructions

### 1. Generate SSL Certificates

Before running the server, you need to generate SSL certificates:

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

### 2. Build and Run

```bash
# Build the server
make

# Run the server
./.bin/tinyserver
```

### 3. Test the Server

Open your browser and navigate to:
- `https://localhost:8080`
- `https://localhost:8080/index.html`

**Note:** You'll get a security warning because it's a self-signed certificate. This is normal for development.

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