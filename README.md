# RCOM Lab 1

## Compilation

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
./bin/main /tmp/ttyS11 9600 rx
```

### Terminal 3: Transmitter

```bash
./bin/main /tmp/ttyS10 9600 tx
```

Start the receiver before the transmitter.

## Custom Files

```bash
./bin/main /tmp/ttyS11 9600 rx received.pdf
./bin/main /tmp/ttyS10 9600 tx original.pdf
```

## Check File Integrity

```bash
make check-integrity
```

For custom files:

```bash
make check-integrity ORIGINAL=original.pdf RECEIVED=received.pdf
```
