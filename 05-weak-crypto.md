# Breaking Weak Crypto

> Part of the [Systems & Network Security Lab](README.md) — see the main README for the overview and the other tracks.

A file was encrypted with a weak, home-grown XOR-stream cipher. I recovered it with a **known-plaintext attack**.

A PDF always begins with the bytes `%PDF-1.x`. XORing that known header against the first ciphertext bytes recovers the seed key, and the keystream advances by a simple modular multiply — so once the seed is known, the whole file falls out. See [`code/known_plaintext_decrypt.c`](code/known_plaintext_decrypt.c).

The lesson is the oldest one in cryptography: **never roll your own.** A predictable header plus a weak keystream is all it takes.


---

## Ethical Notice

All testing was performed in an authorized, isolated lab environment for defensive research only. Do not reproduce any of this against systems you do not own or have explicit permission to test.

## License

MIT — see [`LICENSE`](LICENSE). Copyright © 2025 Vansh Bhardwaj.
