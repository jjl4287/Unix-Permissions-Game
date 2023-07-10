# Unix Permissions Game

The Unix Permissions Game is an educational tool for practicing and understanding Unix file permissions. It's designed to operate as a client-server model, where the server generates Unix permission questions, and the client interacts with users to answer these questions.

## Table of Contents

- [How It Works](#how-it-works)
  - [Client-Side](#client-side)
  - [Server-Side](#server-side)
  - [Socklib.c](#socklibc)
- [How to Use](#how-to-use)
- [Command-line Arguments](#command-line-arguments)
- [Limitations](#limitations)
- [Compatibility](#compatibility)
- [Acknowledgments](#acknowledgments)
- [License](#license)

## How It Works

This program uses a server-client architecture. The client sends requests to the server and receives responses. The server, meanwhile, listens on a specific port for incoming connections, processes requests, and sends back the appropriate responses.

### Client-Side

The client part of the application is responsible for:

- Handling the user input and output.
- Establishing a connection with the server.
- Forming the request based on user input and sending it to the server.
- Receiving the response from the server, parsing it, and presenting it to the user.
- Calculating checksums for verification of the response's integrity.

### Server-Side

1. **Initialization and Connection Acceptance**: The server is initialized and starts to listen on port 31337. When a client makes a request to connect, the server accepts the connection, and it receives the incoming data from the client.
2. **Request Processing**: Depending on the content of the request, the server takes different actions. If the request contains the answer to a question, the server evaluates the answer, updates the user's record, and sends back the necessary response.
3. **Time Delay Management**: The server keeps track of a delay time for each user, which is the waiting time before the user can receive a new question. This delay time is updated based on the user's previous activity and current request.
4. **Question Generation**: The server generates Unix permission questions and sends them to the client upon request. The Unix permission questions are modeled after the Unix file system permissions, represented in the format of 'rwxrwxrwx'.
5. **User Record Management**: The server keeps track of each user's progress in a directory specifically for that user. It records the number of questions attempted and the time delay for the next question.

### Socklib.c

This file contains utility functions for establishing network connections. The library helps create server and client sockets. The server uses `make_server_socket()` to start listening for incoming connections, and the client uses `connect_to_server()` to establish a connection with the server.

## How to Use

The Unix Permissions Game is a command-line application. You can compile and run it using a C compiler.

1. Compile the `client.c` and `server.c` files using your preferred C compiler.

```bash
gcc client.c -o client
gcc server.c -o server
```
2. Run the server program.

```bash
./urevserv
```
3. Run the client program with the appropriate arguments.

```bash
./urev username option
```

## Command-line Arguments

The client program accepts a series of command-line arguments that modify the behavior of the program:

- `-4` : Activates 4-tier mode, regardless of progress.

- `-d` : Activates demo mode. In this mode, the client will not connect to the server to send or retrieve data. This is useful for testing or for previewing the questions without the requirement of a network connection.

- `-l` : Enables a mode that allows backspace and doesn't evaluate until ENTER is pressed.

- `-m` : Activates mini mode, reducing the number of problems to 6 and cutting the time delay in half.

- `-M` : Activates "more" mode, increasing the number of problems to 24 and doubling the time delay.

- `-T` : Specifies a value between 4 and 37 that deducts that number of minutes from the session delay. For example, `-T 10` would subtract 10 minutes from the time delay.

- `-h` : Displays basic usage information and the help page.

- `-V` : Displays current version and author information.


## Compatibility

The Unix Permissions Game has been tested and confirmed to work on Unix-based systems, including various distributions of Linux and macOS. Windows compatibility has not been tested as of the time of writing.

## Limitations

The Unix Permissions Game is a relatively simple game, and it comes with a number of limitations:

1. It does not currently support multiple simultaneous players. If multiple players attempt to play at the same time, their results may interfere with each other.

2. There is no built-in way to reset the game or clear a player's progress. This must be done manually by modifying or deleting the user's file on the server.

3. There is no error-checking for input values. For example, if the `-t` argument is given a non-integer value, the behavior is undefined.

4. There is no scoring or win condition. The game simply continues indefinitely, generating and asking new questions until the player chooses to stop.

## Acknowledgments

This project was developed by Jakob Langtry. 

## License

see LICENSE file
