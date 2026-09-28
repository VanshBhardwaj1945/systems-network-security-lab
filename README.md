# Systems & Network Security Lab

A VirtualBox sandbox where I stand up defenses and then attack them. It began as network segmentation and a Slowloris denial-of-service test, and grew into a set of **attack-and-defend tracks** that run from the network layer down to the stack. Each track has its own write-up with setup, evidence, and the fixes.

- **Type:** Personal security lab (authorized, isolated sandbox only)
- **Role:** Lab builder & test executor
- **Date:** 2025

## Tracks

| # | Track | What it covers |
|---|-------|----------------|
| 1 | [Network Segmentation & Access Control](01-network-segmentation.md) | A multi-subnet network behind a pfSense firewall, an access-control matrix enforced with router- and host-level rules, and validation with scanning + packet captures. |
| 2 | [Slowloris Application-Layer DoS](02-slowloris-dos.md) | A controlled low-bandwidth DoS against Apache, then detecting it in Splunk where volume alerts miss it. |
| 3 | [Credential Attacks on SSH](03-ssh-credential-attacks.md) | How weak/reused SSH passwords fall to a dictionary — with Metasploit and a small C tool I wrote — and the fixes. |
| 4 | [Memory Safety: Buffer Overflow & ASLR](04-buffer-overflow-aslr.md) | A classic stack buffer overflow analyzed in `gdb`, and the math for why ASLR defeats a return-address overwrite. |
| 5 | [Breaking Weak Crypto](05-weak-crypto.md) | Recovering a file from a home-grown XOR-stream cipher with a known-plaintext attack. |

## What I took away

- **Defense in depth is the through-line.** Segment the network, then assume something still gets through and plan for it.
- **A control you haven't tested is a guess.** I validated every firewall rule with active scanning and packet captures, not assumptions.
- **Detection matters as much as prevention.** The Slowloris attack was invisible to volume alerts but obvious in the right Splunk search.
- **The cheapest attacks are the most common.** Weak SSH passwords and home-grown crypto fall with almost no effort — so the boring fixes (keys, strong passwords, real crypto) are the high-value ones.
- **Low-level safety is layered too.** ASLR, stack canaries, and a non-executable stack each raise the bar on memory bugs.

## Tools & tech

VirtualBox · pfSense · iptables · Splunk · Nmap / Zenmap · Wireshark · tcpdump · Kali · Metasploit · Slowloris · gdb · C (libssh2)

## Code

Small, self-contained programs from the lab, all MIT-licensed and built and run **only** inside the isolated sandbox against my own systems:

- [`code/ssh_dictionary_check.c`](code/ssh_dictionary_check.c) — SSH dictionary login checker (`libssh2`). Authorized / educational use only.
- [`code/known_plaintext_decrypt.c`](code/known_plaintext_decrypt.c) — known-plaintext decryptor for a weak XOR-stream cipher.

## Ethical Notice

All testing was performed in an authorized, isolated lab environment for defensive research only. Do not reproduce any of this against systems you do not own or have explicit permission to test.

## License

Released under the **MIT License** — see [`LICENSE`](LICENSE). Copyright © 2025 Vansh Bhardwaj. The offensive code here is published for education and for testing systems you own or are explicitly authorized to test — nothing else.
