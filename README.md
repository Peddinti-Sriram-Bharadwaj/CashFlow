# Cashflow 

## Project Overview
**Cashflow** is a robust financial management application built entirely in C, utilizing POSIX threading library, keeping security in mind. Currently, it is supported on macos only.

## Architecture
The application is divided into three distinct layers for logical convenience:
<div style="text-align: center;">
    <img src="https://github.com/user-attachments/assets/b6de2ab6-2f3f-4037-bfae-6147428c7891" alt="Architecture Diagram" height="600">
</div>


### 1. Client Side
- **User Authentication**: The client-side code allows users to authenticate and log in.
- **Request Handling**: Users can send requests to the server, formatted as structures that contain operation codes.

### 2. Server Side
The server side consists of two layers:
- **Request Handler**: Receives requests from the client and determines the appropriate action.
- **Operation Handler**: Executes the requested operations based on the client's request.

### Communication Flow
1. The client sends a request in the form of a structure containing:
   - Operation code
   - Client socket file descriptor
   - Username (if necessary)

2. A switch statement on the server creates a corresponding service-level thread that handles the operation.

3. The operation thread:
   - Performs mandatory locking on the required file to ensure data integrity.
   - Executes atomic operations on the file.
   - Unlocks the file upon completion.

4. If additional communication is required, the operation thread interacts with the client using necessary structures.

## Security Features
- **Password Hashing**: Utilizes `libsodium` for hashing passwords, ensuring that even developers cannot retrieve user passwords.
- **Console Security**: Employs `termios` to modify console settings, preventing password visibility during runtime.
- **Communication Protocol**: Uses AF_UNIX stream sockets for secure and efficient communication between client and server.

## Installation
To set up Cashflow on your local machine (macOS), follow these steps:

1. **Clone the repository**:  
   ```bash
   git clone https://github.com/Peddinti-Sriram-Bharadwaj/CashFlow
   ```

	1. Install dependencies:
    
    Install `libsodium` and `termios` using Homebrew:
```bash
   brew install libsodium
```
2. **Compile the code**:
 
Navigate into each directory (`./server`, `./admin`, `./customer`, `./manager`, `./employee`) and run `make` to create object files:
```bash
cd server && make
cd ../admin && make
cd ../customer && make
cd ../manager && make
cd ../employee && make
```

Usage

To run the application:

1.	head to ./admin folder
2. 	run the addAdmin.out file
```bash
 cd admin && ./addAdmin.out
```
4. 	add an admin role
5.	interrupt the code, and proceed as follows.
6.	Open two terminal windows.
7.	In one terminal, navigate to the server directory and run:
 ```bash
 cd server && ./server.out
```
  7.  In another terminal, navigate to the root directory and run:
 
 ```bash
 ./welcome.out
```
Notes on Execution
	•	The application uses `execvp` to run functionalities for different roles, which are located in separate files.
	•	Ensure that your libsodium installation path matches `/opt/homebrew/Cellar/libsodium/1.0.20/include` for header files and `-L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium` for linking.

## Contributing

Contributions are welcome! If you have suggestions or improvements, please submit a pull request or open an issue for discussion.


## License

This project is licensed under the MIT License. For more details, please refer to the LICENSE file in this repository.
