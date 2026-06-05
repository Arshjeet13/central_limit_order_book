# ironbook — BOE Gateway + Matching Engine

A high-performance C++ order book and matching engine with a partial Cboe Binary Order Entry (BOE) wire protocol gateway layer.

## Project Structure

```
ironbook/
├── boe/
│   └── messages.hpp      — packed BOE message structs
├── engine/
│   ├── engine.hpp        — matching engine interface
│   └── engine.cpp        — price-time priority matching logic
├── gateway/
│   ├── session.hpp       — session state + recv_exact
│   ├── session.cpp       — TCP recv loop, message framing
│   ├── parser.hpp        — message dispatch
│   ├── parser.cpp        — raw bytes → BOE structs → handlers
│   ├── handlers.hpp      — handler declarations
│   └── handlers.cpp      — business logic, engine calls, responses
└── main.cpp              — TCP server, accept loop
```

---

## Protocol Overview

The gateway implements a subset of the Cboe US Options BOE specification (v2.11.68). BOE is a binary order entry protocol — fixed-size packed structs, little-endian encoding, no text parsing.

### Implemented Messages

**Client → Gateway (inbound):**

| Message | Type | Description |
|---|---|---|
| Login Request | 0x37 | First message on connect, establishes session |
| Logout Request | 0x02 | Graceful disconnect |
| Client Heartbeat | 0x03 | Keepalive when idle >1 second |
| New Order | 0x38 | Submit limit or market order |
| Cancel Order | 0x39 | Cancel single order by OrigClOrdID |

**Gateway → Client (outbound):**

| Message | Type | Description |
|---|---|---|
| Login Response | 0x24 | Accepts or rejects login |
| Logout | 0x08 | Confirms disconnect or server-initiated close |
| Server Heartbeat | 0x09 | Keepalive when idle >1 second |
| Replay Complete | 0x13 | Signals end of replay (sent immediately, no replay implemented) |
| Order Acknowledgment | 0x25 | Order accepted, resting on book |
| Order Rejected | 0x26 | Order failed validation |
| Order Cancelled | 0x2A | Cancel succeeded |
| Cancel Rejected | 0x2B | Cancel failed |
| Order Execution | 0x2C | Fill occurred — sent to both sides |

---

## Design Decisions

### Binary Protocol over FIX

FIX is the industry-standard order entry protocol, but it is text-based (`tag=value|tag=value|...`) with 30 years of accumulated complexity — multiple versions, per-counterparty data dictionaries, repeating groups, session management state machines, and a text parser before a single order reaches the matching engine.

BOE is Cboe's proprietary binary protocol. Messages are packed structs. Parsing is a single `reinterpret_cast` from a byte buffer. The entire parser fits in a switch statement. This makes BOE the correct choice for a project whose goal is demonstrating low-latency infrastructure, not implementing a universal broker connector.

### `#pragma pack(push, 1)` on all BOE structs

By default the C++ compiler adds padding between struct fields for alignment — a `uint8_t` followed by a `uint32_t` may have 3 bytes of padding inserted between them. This is invisible in normal code but fatal for protocol parsing: the struct layout would no longer match the wire format.

`#pragma pack(push, 1)` instructs the compiler to pack fields with no padding, so field offsets in the struct match byte offsets on the wire exactly. This makes casting a raw byte buffer directly to a struct pointer both correct and safe.

### `uint8_t` for byte buffers, not `char`

`char` is semantically a character — text, ASCII. When reading raw binary protocol data off a network socket, the data is not text. `uint8_t` (unsigned 8-bit integer) correctly represents a raw byte.

The practical difference: if a byte value is `0xBA`, a `signed char` interprets it as `-70`. A `uint8_t` keeps it as `186`. Protocol fields are compared against hex constants — unsigned is always correct here.

Additionally, `uint8_t*` arithmetic is clean and requires no casts: `buf += n` advances by exactly `n` bytes. `char*` requires the same math but `void*` buffers need explicit casts.

### `recv_exact` wrapper around `recv`

TCP is a stream protocol — the kernel delivers bytes as they arrive off the network, with no concept of message boundaries. A single `recv(fd, buf, 40, 0)` call may return 1 byte, 12 bytes, or 40 bytes depending on what the kernel has buffered at that moment.

`recv_exact` loops until exactly `n_bytes` have been accumulated:

```cpp
bool recv_exact(int fd, uint8_t* buf, int n_bytes) {
    while (n_bytes > 0) {
        int received = recv(fd, buf, n_bytes, 0);
        if (received <= 0) return false;  // disconnect or error
        n_bytes -= received;
        buf += received;
    }
    return true;
}
```

Without this, the first `recv` call might return a partial message and the parser would see garbage.

### Two-step message framing

BOE messages have variable length. You cannot know how many bytes to read until you have seen the `MessageLength` field. The framing strategy is therefore:

1. Read exactly 4 bytes: `StartOfMessage` (2) + `MessageLength` (2)
2. Validate `StartOfMessage == 0xBABA`
3. Read `MessageLength - 2` more bytes (the length field counts itself but not StartOfMessage)
4. Assemble the full buffer and dispatch to parser

This is the standard approach for length-prefixed binary protocols.

### No `ntohs`/`htons` for BOE fields

`ntohs` and `htons` convert between network byte order (big-endian) and host byte order. They apply to IP and TCP header fields — port numbers, IP addresses — not application payload data.

TCP transmits payload bytes exactly as given, preserving their order. Since BOE is little-endian and x86 is little-endian, no conversion is needed. The spec explicitly states: "All binary values are in Little Endian (used by Intel x86 processors), and not network byte order."

A `from_le<T>()` template exists for future portability on big-endian architectures but is a no-op on x86.

### `Session` struct for per-connection state

Each TCP connection requires state that persists between messages — whether the client is logged in, the outbound sequence number counter, the client's username. This state cannot live inside individual handlers (reset on every call) or as globals (breaks with multiple clients).

`Session` is the memory that connects successive messages from the same client:

```cpp
struct Session {
    int      fd{-1};
    bool     logged_in{false};
    uint32_t next_seq_num{1};
    char     username[4]{};
};
```

Handlers receive a `Session&` reference and read/write it. The session is created when a client connects and destroyed when they disconnect.

### Separate sequence numbers per session

Each client's outbound messages are independently sequenced starting at 1. A global counter would cause gaps in individual session streams — client A getting sequences 1, 3, 5 and client B getting 2, 4, 6 — both seeing apparent missed messages.

### `ClOrdID` vs `OrigClOrdID`

`ClOrdID` is a client-assigned identifier for a new order. It must be unique among currently live orders.

`OrigClOrdID` is a reference to an existing order's `ClOrdID`, used in Cancel Order messages. Cancel Order has no `ClOrdID` of its own — a cancel creates nothing new in the system, so it needs no new identity. It only references what it wants to remove.

### `make_message<T>()` template for struct construction

`sizeof(T)` cannot appear in `T`'s own in-class member initializers — the class is incomplete at that point and the compiler doesn't yet know its total size. Constructors work (the class is complete by the time the constructor body executes) but adding constructors to POD structs removes their POD status, which matters for safe `reinterpret_cast` from byte buffers.

A free function template solves this cleanly:

```cpp
template<typename T>
T make_message() {
    T msg{};
    msg.message_length = static_cast<uint16_t>(sizeof(T) - 2);
    return msg;
}
```

The `-2` excludes `StartOfMessage` which the spec explicitly does not count in `MessageLength`.

### Order Execution sent to both sides

When a fill occurs, two clients are affected — the aggressor (incoming order that crossed) and the resting order's owner. Both receive an `order_execution` message with the same `ExecID` and `LastPx` but different `ClOrdID` and `BaseLiquidityIndicator`:

- Aggressor: `BaseLiquidityIndicator = 'R'` (Removed Liquidity)
- Resting: `BaseLiquidityIndicator = 'A'` (Added Liquidity)

`LeavesQty = 0` signals the order is fully filled. Non-zero means a partial fill with the remainder still resting on the book.

### Replay Complete always sent immediately

The spec requires a `replay_complete` message after login to signal that any missed messages have been replayed. The client must not send orders until it receives this.

This gateway has no replay mechanism — no persistent message log between sessions. `replay_complete` is sent immediately after `login_response` with no messages in between. This is the valid "start of day / no prior state" case defined in the spec.

---

## BOE Message Framing Reference

```
Bytes 0-1:  StartOfMessage  — always 0xBABA
Bytes 2-3:  MessageLength   — byte count from here to end (excludes StartOfMessage)
Byte  4:    MessageType     — identifies the message
Byte  5:    MatchingUnit    — 0 for all client→gateway messages
Bytes 6-9:  SequenceNumber  — 0 for session messages, incrementing for application messages
Bytes 10+:  Message-specific fields
```

## Optional Fields / Bitfields

Some messages (New Order) have optional fields appended after fixed fields. A bitfield byte (or bytes) specifies which optional fields are present. Each bit maps to a specific field defined in the spec. Fields are appended in bit order — lowest bit of first bitfield first.

For this gateway, New Order always uses:

```
Bitfield1 = 0x14  →  Price (bit 0x04) + OrdType (bit 0x10)
Bitfield2 = 0x41  →  Symbol (bit 0x01) + Capacity (bit 0x40)
Optional fields appended: Price (8B), OrdType (1B), Symbol (8B), Capacity (1B)
```

---

## What Is Not Implemented

- Replay / gap fill on reconnect
- Heartbeat timers (planned step 2)
- Multiple concurrent clients (planned step 3)
- Mass cancel
- Modify Order
- Endian portability on big-endian architectures (stubbed, `from_le<T>()` is a no-op)
- BOE optional return fields (NumberOfReturnBitfields always 0)