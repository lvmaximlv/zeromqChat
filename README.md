# zeromqChat

zeromqChat is simple client/server chat using ZeroMQ

## Requirements

 - [ZerMQ](https://zeromq.org/download/) Tested with 4.3.2

 ## Installation

 1.	Install ZeroMQ. `$ apt-get install libzmq3-dev`
 2.	Install server:

```bash
cd Server/
make
```
 3. Install client:

```bash
cd Client/
make
```

## Usage

### Server

	You can start server as a daemon or in interactive mode.
	Go to server dir: `$ cd  Server/`

#### interactive mode

	To start server in interactive mode use `$ ./server -i`

#### daemon mode

	To start sever in daemon mod use `$ ./server -d`

### Client

	Go to client dir: `$ cd Client/`

#### start client

1.	start client use `$ ./client`
2.	enter your username or leave field empty for 'User' and press Enter;
3. 	enter server ip address or leave empty for 'localhost' and press Enter;
4.	start chatting!

