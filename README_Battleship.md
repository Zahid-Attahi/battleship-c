# Battleship 3

A terminal-based Battleship game written in C. Play solo against a computer opponent, or challenge another player over a real network connection using a custom TCP protocol.

## Features

- **Single-player mode** — play against a computer that randomly places and fires ships
- **Two-player network mode** — connect two machines over TCP and play against each other in real time
- 10x10 board with all 5 classic ships (Carrier, Battleship, Cruiser, Submarine, Destroyer)
- Interactive ship placement with input validation
- Live board display after every turn showing your ships and your shots
- Manual memory management — all grids allocated and freed explicitly

## How to Compile

```bash
gcc battleship.c -o battleship -lm
```

## How to Run

**Single-player (vs computer):**
```bash
./battleship
```

**Two-player — start the server:**
```bash
./battleship 8080
```

**Two-player — connect as client:**
```bash
./battleship 192.168.1.10 8080
```

## How to Play

**Placing ships:**

Enter a start row, end row, and column in compact format:
```
AE4   → rows A through E, column 4 (vertical, 5 squares)
AA2   → row A only, column 2 (horizontal, 1 square — use for shorter ships)
```

**Taking shots:**

Enter a row letter and column number:
```
B4   → row B, column 4
```

**Board display:**

| Symbol | Meaning |
|--------|---------|
| `.` | Empty / not tried |
| `X` | Hit |
| `O` | Miss |
| `CV` `BB` `CR` `SS` `DD` | Your ships |

## Ships

| Ship | Size |
|------|------|
| Carrier | 5 |
| Battleship | 4 |
| Cruiser | 3 |
| Submarine | 3 |
| Destroyer | 2 |

## Network Protocol

The two-player mode uses a simple custom TCP protocol over raw sockets:

- `READY` — handshake to confirm both players are connected
- `SHOT <row> <col>` — send a shot to the opponent
- `RESULT HIT / MISS / WIN` — opponent replies with the outcome

## Built With

- C (C99)
- POSIX sockets (`sys/socket.h`, `netdb.h`)
- No external libraries

## Author

Solo project — covers single-player AI, dynamic memory management, and networked multiplayer from scratch.
