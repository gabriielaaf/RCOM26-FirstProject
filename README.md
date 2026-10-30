# RCOM Lab 1: Data Link Protocol

Implementation of a reliable file transfer protocol over a serial connection, developed in C for the Computer Networks course at FEUP (2026/2027).

The project implements the Stop and Wait ARQ protocol, including frame acknowledgements, error detection, retransmissions, byte stuffing and connection management.

## Compilation

Compile the project from the root directory:

```bash
make
```

## How to Run

Open three terminals in the project directory.

### Terminal 1: Virtual Cable

```bash
./bin/cable
```

### Terminal 2: Receiver

```bash
./bin/main /tmp/ttyS11 9600 rx received.gif
```

### Terminal 3: Transmitter

```bash
./bin/main /tmp/ttyS10 9600 tx original.gif
```

Start the receiver before the transmitter.

## File Integrity

Verify that the received file is identical to the original:

```bash
make check-integrity ORIGINAL=original.pdf RECEIVED=received.pdf
```

A successful comparison displays:

```text
FILE INTEGRITY: PASS
```

## Testing

The virtual cable allows you to simulate connection failures and transmission errors.

Enter the following commands in the virtual cable terminal:

| Command | Description |
| :--- | :--- |
| `on` | Enable communication |
| `off` | Disable communication |
| `ber 0.00001` | Introduce random bit errors |
| `ber 0` | Disable random bit errors |
| `help` | Display available commands |

The protocol automatically handles timeouts, retransmissions and corrupted frames.

## Clean

Remove compiled files:

```bash
make clean
```