# Memory Safety: Buffer Overflow & ASLR

> Part of the [Systems & Network Security Lab](README.md) — see the main README for the overview and the other tracks.

A C program copies attacker-controlled input into a **256-byte** stack buffer without checking the length, so anything longer than 256 bytes runs off the end of the buffer and into the rest of the stack frame.

## Stack analysis

In `gdb` I mapped the frame to see exactly what an overflow would reach: the buffer itself, the saved base pointer (RBP), and — sitting just past them — the **saved return address** the function jumps to when it finishes. Recording the stack pointer, base pointer, buffer address, and return address showed how many bytes of overflow it takes to reach and overwrite that return address.

> Scope note: this track is the **stack analysis and the reasoning**, not a working exploit — the remote lab dropped before the exploitation step, so I did not land control of execution.

## Why ASLR defeats it

Overwriting a return address only works if you know a **fixed** address to jump to. Address-space layout randomization (ASLR) moves things on every run, so input that assumes one address usually jumps somewhere invalid and the program just crashes.

The math makes it concrete. If only the low **16 bits** of the address are randomized:

- possibilities = 2¹⁶ = **65,536**
- a single guess is right about **1 in 65,536** (≈ 0.0015%)
- at 10 tries per second that is ≈ **1 hour 49 minutes** to brute-force

And 16 bits is the *weak* case. Full randomization, plus **stack canaries** and a **non-executable stack**, moves this from "annoying" to "not worth it." The takeaway is defense in depth for memory safety: bounds-checked code first, then ASLR, canaries, and NX behind it.


---

## Ethical Notice

All testing was performed in an authorized, isolated lab environment for defensive research only. Do not reproduce any of this against systems you do not own or have explicit permission to test.

## License

MIT — see [`LICENSE`](LICENSE). Copyright © 2025 Vansh Bhardwaj.
