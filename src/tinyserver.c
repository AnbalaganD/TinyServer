/*
 * =============================================================================
 * TINY SSL SERVER - BEGINNER'S GUIDE TO C NETWORK PROGRAMMING
 * =============================================================================
 * This program creates a secure HTTPS server using OpenSSL library.
 * It demonstrates:
 * 1. Socket programming (network communication)
 * 2. SSL/TLS encryption (secure communication)
 * 3. Server-client architecture
 * 4. Certificate-based authentication
 * =============================================================================
 */

// =============================================================================
// HEADER FILES (INCLUDES)
// =============================================================================
// Header files contain function declarations and constants we need
// Think of them as "instruction manuals" for different capabilities

#include <stdio.h>      // Standard Input/Output: printf, perror, etc.
#include <stdlib.h>     // Standard Library: exit, EXIT_FAILURE, etc.
#include <string.h>     // String functions: strlen, etc.
#include <unistd.h>     // Unix standard: close, etc.
#include <sys/socket.h> // Socket functions: socket, bind, listen, accept
#include <arpa/inet.h>  // Internet operations: htons, INADDR_ANY
#include <openssl/ssl.h> // OpenSSL SSL functions
#include <openssl/err.h> // OpenSSL error handling

// =============================================================================
// CONSTANTS (DEFINES)
// =============================================================================
// #define creates constants that never change during program execution
// Using constants makes code easier to read and maintain

#define PORT 8080           // Port number server listens on (like a door number)
#define CERT_FILE "server.crt"  // Server's public certificate file
#define KEY_FILE "server.key"   // Server's private key file  
#define CA_FILE "ca.crt"        // Certificate Authority file (trusted root)
#define BUFFER_SIZE 4096        // Size of buffer for HTTP requests/responses

// Global flag to enable/disable TLS (can be set via command line)
int use_tls = 1;  // 1 = use TLS/SSL, 0 = plain HTTP

// =============================================================================
// FUNCTION DECLARATIONS AND EXPLANATIONS
// =============================================================================

/*
 * FUNCTION: init_openssl
 * PURPOSE: Initialize OpenSSL library for use
 * RULE: Always initialize libraries before using them
 * WHY: OpenSSL needs to set up internal data structures and error messages
 */
void init_openssl() {
    // Load human-readable error messages (instead of just error codes)
    SSL_load_error_strings();
    
    // Initialize SSL algorithms (encryption methods)
    // This sets up the cryptographic functions OpenSSL will use
    OpenSSL_add_ssl_algorithms();
}

/*
 * FUNCTION: cleanup_openssl  
 * PURPOSE: Clean up OpenSSL when we're done
 * RULE: Always clean up resources you initialize
 * WHY: Prevents memory leaks and properly shuts down the library
 */
void cleanup_openssl() {
    // Clean up algorithm data structures
    EVP_cleanup();
}

/*
 * FUNCTION: create_context
 * PURPOSE: Create SSL context (like a "settings container" for SSL)
 * RETURNS: SSL_CTX pointer (context object)
 * RULE: Always check if functions return NULL (failure)
 * WHY: SSL context holds all the configuration for SSL connections
 */
SSL_CTX *create_context() {
    // Declare variables at the top (C89/C90 rule)
    const SSL_METHOD *method;  // SSL method (which SSL version to use)
    SSL_CTX *ctx;             // SSL context (our "settings container")

    // Get SSL method - SSLv23 means "use best available SSL/TLS version"
    method = SSLv23_server_method();

    // Create new SSL context using the method
    ctx = SSL_CTX_new(method);
    
    // ALWAYS check if function succeeded (returned non-NULL)
    if (!ctx) {
        // perror prints system error message to stderr
        perror("Unable to create SSL context");
        
        // Print OpenSSL-specific error messages
        ERR_print_errors_fp(stderr);
        
        // Exit program with failure code
        exit(EXIT_FAILURE);
    }

    // Return the successfully created context
    return ctx;
}

/*
 * FUNCTION: configure_context
 * PURPOSE: Load certificates and configure SSL settings
 * PARAMETER: ctx - SSL context to configure
 * RULE: Always validate certificates and keys match
 * WHY: SSL needs certificates to prove server identity and encrypt data
 */
void configure_context(SSL_CTX *ctx) {
    
    // Load server certificate file (public key that clients can verify)
    // SSL_FILETYPE_PEM means certificate is in PEM format (text-based)
    if (SSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM) <= 0) {
        // If loading fails, print error and exit
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Load server private key file (secret key for decryption)
    // This MUST match the certificate loaded above
    if (SSL_CTX_use_PrivateKey_file(ctx, KEY_FILE, SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Load Certificate Authority file (trusted root certificate)
    // This is used to verify client certificates
    if (SSL_CTX_load_verify_locations(ctx, CA_FILE, NULL) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Set verification mode - require client to present valid certificate
    // SSL_VERIFY_PEER: verify the client certificate
    // SSL_VERIFY_FAIL_IF_NO_PEER_CERT: fail if client has no certificate
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
}

/*
 * FUNCTION: main
 * PURPOSE: Entry point of the program - where execution starts
 * PARAMETERS: argc (argument count), argv (argument values)
 * RULE: main function must return int (0 = success, non-zero = error)
 * WHY: Operating system needs to know if program succeeded or failed
 */
int main(int argc, char **argv) {
    
    // =============================================================================
    // COMMAND LINE ARGUMENT PARSING
    // =============================================================================
    
    // Check for --no-tls flag to disable SSL/TLS
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-tls") == 0) {
            use_tls = 0;
            printf("TLS disabled - running as plain HTTP server\n");
            break;
        }
    }
    
    // =============================================================================
    // VARIABLE DECLARATIONS
    // =============================================================================
    // In C, declare all variables at the beginning of a block (C89/C90 rule)
    
    int sock;                    // Socket file descriptor (like a handle)
    struct sockaddr_in addr;     // Address structure (IP + port info)
    unsigned int len = sizeof(addr); // Size of address structure
    SSL_CTX *ctx = NULL;        // SSL context (our SSL settings) - NULL if no TLS
    SSL *ssl = NULL;            // SSL connection object - NULL if no TLS

    // =============================================================================
    // INITIALIZATION PHASE
    // =============================================================================
    
    // Only initialize SSL if TLS is enabled
    if (use_tls) {
        // Step 1: Initialize OpenSSL library
        init_openssl();
        
        // Step 2: Create SSL context (settings container)
        ctx = create_context();
        
        // Step 3: Configure SSL context (load certificates)
        configure_context(ctx);
        
        printf("TLS enabled - running as HTTPS server\n");
    } else {
        printf("TLS disabled - running as HTTP server\n");
    }

    // =============================================================================
    // SOCKET CREATION AND SETUP
    // =============================================================================
    
    // Create socket: AF_INET = IPv4, SOCK_STREAM = TCP, 0 = default protocol
    // Returns file descriptor (integer handle) for the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR to allow reusing the address immediately
    // This prevents "Address already in use" error when restarting server
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Unable to set socket options");
        exit(EXIT_FAILURE);
    }

    // Configure address structure (where server will listen)
    addr.sin_family = AF_INET;        // IPv4 address family
    addr.sin_port = htons(PORT);      // Convert port to network byte order
    addr.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces

    // Bind socket to address (claim the port)
    // This is like putting your name on a mailbox
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections (queue size = 1)
    // This makes the socket ready to accept incoming connections
    if (listen(sock, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    // Print status message to let user know server is ready
    printf("Server listening on port %d\n", PORT);

    // =============================================================================
    // MAIN SERVER LOOP
    // =============================================================================
    // while(1) creates infinite loop - server runs until manually stopped
    
    while(1) {
        
        // Accept incoming connection from client
        // This blocks (waits) until a client connects
        // Returns new socket for this specific client
        int client = accept(sock, (struct sockaddr*)&addr, &len);
        if (client < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        if (use_tls) {
            // =============================================================================
            // TLS/SSL MODE - Encrypted communication
            // =============================================================================
            
            // Create new SSL object for this client connection
            ssl = SSL_new(ctx);
            
            // Associate SSL object with client socket
            SSL_set_fd(ssl, client);

            // Perform SSL handshake with client
            if (SSL_accept(ssl) <= 0) {
                ERR_print_errors_fp(stderr);
            } else {
                // SSL handshake successful - communicate securely
                char request_buffer[BUFFER_SIZE] = {0};
                int bytes = SSL_read(ssl, request_buffer, sizeof(request_buffer) - 1);
                
                if (bytes > 0) {
                    request_buffer[bytes] = '\0';
                    printf("Received HTTPS request:\n%s\n", request_buffer);
                    
                    // Parse and respond with HTTP
                    char method[16] = {0}, url[256] = {0};
                    sscanf(request_buffer, "%s %s", method, url);
                    
                    char html_content[1024];
                    sprintf(html_content,
                        "<!DOCTYPE html><html><head><title>Tiny SSL Server</title></head>"
                        "<body><h1>Secure HTTPS Server!</h1><p>Method: %s</p><p>URL: %s</p></body></html>",
                        method, url);
                    
                    char http_response[BUFFER_SIZE];
                    sprintf(http_response,
                        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s",
                        strlen(html_content), html_content);
                    
                    SSL_write(ssl, http_response, strlen(http_response));
                }
            }
        } else {
            // =============================================================================
            // PLAIN HTTP MODE - No encryption
            // =============================================================================
            
            // Read plain HTTP request directly from socket
            char request_buffer[BUFFER_SIZE] = {0};
            int bytes = read(client, request_buffer, sizeof(request_buffer) - 1);
            
            if (bytes > 0) {
                request_buffer[bytes] = '\0';
                printf("Received HTTP request:\n%s\n", request_buffer);
                
                // Parse and respond with HTTP
                char method[16] = {0}, url[256] = {0};
                sscanf(request_buffer, "%s %s", method, url);
                
                char html_content[1024];
                sprintf(html_content,
                    "<!DOCTYPE html><html><head><title>Tiny HTTP Server</title></head>"
                    "<body><h1>Plain HTTP Server!</h1><p>Method: %s</p><p>URL: %s</p></body></html>",
                    method, url);
                
                char http_response[BUFFER_SIZE];
                sprintf(http_response,
                    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s",
                    strlen(html_content), html_content);
                
                // Send plain HTTP response directly to socket
                write(client, http_response, strlen(http_response));
            }
        }

        // =============================================================================
        // CONNECTION CLEANUP
        // =============================================================================
        
        // Clean up SSL resources only if TLS is enabled
        if (use_tls && ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        
        // Close client socket (always needed)
        close(client);
        
        // Loop continues to accept next client...
    }

    // =============================================================================
    // PROGRAM CLEANUP (This code never runs due to infinite loop above)
    // =============================================================================
    // In a real server, you'd have signal handlers to break the loop gracefully
    
    close(sock);           // Close server socket
    
    // Clean up SSL resources only if TLS was enabled
    if (use_tls && ctx) {
        SSL_CTX_free(ctx);     // Free SSL context
        cleanup_openssl();     // Cleanup OpenSSL library
    }

    return 0;  // Return success (though this line never executes)
}

/*
 * =============================================================================
 * TCP SEGMENT HEADER ASCII DIAGRAM
 * =============================================================================
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |          Source Port          |       Destination Port        |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                        Sequence Number                        |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    Acknowledgment Number                      |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |  Data |           |U|A|P|R|S|F|                               |
 *   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
 *   |       |           |G|K|H|T|N|N|                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |           Checksum            |         Urgent Pointer        |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    Options                    |    Padding    |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                             data                              |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Flags:
 *   URG: Urgent Pointer field is significant
 *   ACK: Acknowledgment field is significant
 *   PSH: Push Function
 *   RST: Reset the connection
 *   SYN: Synchronize sequence numbers
 *   FIN: No more data from sender
 *
 * =============================================================================
 */

/*
 * =============================================================================
 * PROGRAM FLOW SUMMARY:
 * =============================================================================
 * 1. Initialize OpenSSL library
 * 2. Create and configure SSL context with certificates
 * 3. Create network socket
 * 4. Bind socket to port and start listening
 * 5. Enter infinite loop:
 *    a. Accept client connection
 *    b. Create SSL object for client
 *    c. Perform SSL handshake
 *    d. Read encrypted message from client
 *    e. Send encrypted response to client
 *    f. Clean up client connection
 *    g. Repeat for next client
 * 
 * KEY C PROGRAMMING RULES DEMONSTRATED:
 * - Declare variables at beginning of blocks
 * - Always check return values for errors
 * - Clean up resources you allocate
 * - Use meaningful variable names
 * - Handle errors gracefully
 * - Initialize variables before use
 * =============================================================================
 */