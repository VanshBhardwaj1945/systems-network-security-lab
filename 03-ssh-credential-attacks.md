# Credential Attacks on SSH

> Part of the [Systems & Network Security Lab](README.md) — see the main README for the overview and the other tracks.

Weak or reused passwords are still the most common way into a system, so I showed how little it takes inside the sandbox. Nothing here is an exploit — it is guessing.

## Two approaches, same idea

- **Metasploit** — the `auxiliary/scanner/ssh/ssh_login` module with common username and password lists.
- **A small SSH login checker I wrote in C** with `libssh2` — [`code/ssh_dictionary_check.c`](code/ssh_dictionary_check.c). It reads a wordlist, tries each password for a user over SSH, and stops at the first that authenticates.

## Results

- With a weak password, a plain dictionary logs straight in — **no exploit and no privilege escalation needed**, which is exactly why it matters.
- Each attempt costs roughly 2.5–3.6 seconds over the network, so a one-million-word list is on the order of weeks — but a *weak* password falls in seconds. The security of password auth is entirely the password.

## Defenses

- Disable password auth and require **SSH keys**.
- Enforce strong, unique passwords.
- Rate-limit and lock out repeated failures (`fail2ban`), and **alert** on them.
- Keep SSH off the public internet where you can, or put it behind a bastion / allowlist.

> The screenshots for this track show a real host and recovered credentials, so they are kept out of the repo.


---

## Ethical Notice

All testing was performed in an authorized, isolated lab environment for defensive research only. Do not reproduce any of this against systems you do not own or have explicit permission to test.

## License

MIT — see [`LICENSE`](LICENSE). Copyright © 2025 Vansh Bhardwaj.
