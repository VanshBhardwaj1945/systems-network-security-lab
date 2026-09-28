# Network Segmentation & Access Control

> Part of the [Systems & Network Security Lab](README.md) — see the main README for the overview and the other tracks.

## Part 1 Overview

A repeatable, VirtualBox-based sandbox for implementing, enforcing, and verifying network security policy. The environment models an internal company network and an external attacker network separated by a pfSense virtual router. The goal is to show how router-level and host-level controls combine into defense-in-depth, and how to validate that enforcement with active scanning and packet captures rather than assuming the rules work.

## Sandbox Components

- **Network A (Internal / Company)** — Ubuntu Desktop (server) and Windows XP (workstation)
- **Network B (External / Attacker)** — Kali Linux (attacker / scanner) and Windows 95 (legacy)
- **Router / Firewall** — pfSense virtual appliance connecting Network A and Network B

**Notes & lessons learned**

- Resolved legacy VM patching and NIC misconfiguration issues to keep the environment reproducible.
- Identified policy items that required host-level enforcement when gateway rules alone were insufficient.
- Captured configuration and verification artifacts so the sandbox can be re-created for future testing.

---

## Tools & Technologies Used

| Category | Tools |
|---|---|
| **Virtualization** | VirtualBox (VM creation, NIC binding, snapshots) |
| **Router & firewall** | pfSense (interface configuration, rule authoring, logging) |
| **Operating systems** | Ubuntu, Kali Linux, Windows XP, Windows 95 (legacy) |
| **Network analysis & scanning** | Wireshark, tcpdump, Nmap / Zenmap |
| **Services** | Apache (HTTP), OpenSSH (SSH) |
| **Host hardening** | iptables (Ubuntu) |
| **Testing & validation** | curl, ping, ssh |

---

## Network Topology

<img src="https://i.imgur.com/0PbmIyM.png" alt="Network segmentation topology: two subnets separated by a pfSense router" width="60%">

*A.1 = Ubuntu (server), A.2 = Windows XP (workstation), pfSense = router, B.1 = Kali, B.2 = Windows 95 (legacy).*

---

## Setup (High-Level Steps)

1. Installed Oracle VirtualBox and provisioned VMs for Ubuntu, Kali, Windows XP, and Windows 95.
   <img src="https://i.imgur.com/iG1MSqE.png" alt="VirtualBox VM inventory for the lab" width="60%">

2. Configured pfSense with two interfaces — `LAN` for Network A and `OPT1` for Network B.
   <img src="https://i.imgur.com/ccb1EGK.png" alt="pfSense interface assignment for LAN and OPT1" width="60%">

3. Assigned static IPs and verified NIC-to-subnet mappings.
4. Installed Apache and OpenSSH on the internal server (A.1) and confirmed the services were reachable.
5. Installed Wireshark and Nmap on the attacker and server VMs for traffic capture and scanning.
6. Validated baseline connectivity with `ping`, `curl`, and `ssh`.
7. Documented environment quirks (snapshots + notes) to keep the build reproducible.

---

## Discovery & Baseline Validation

Ran discovery scans and packet captures to establish a clear baseline *before* any policy changes, so post-rule behavior could be compared against a known-good reference.

- Nmap scans from the attacker VM (Kali) to enumerate services and open ports.
- Wireshark / tcpdump captures during `ping`, `curl`, and `ssh` to record normal traffic patterns.
- Baseline evidence saved for before/after comparison.

### Selected Baseline Captures

**Ping and curl from attacker → server**
<img src="https://i.imgur.com/Cfs9b6y.png" alt="Baseline ping and curl from attacker to server" width="45%">

**SSH from attacker → server**
<img src="https://i.imgur.com/Bu5DlBx.png" alt="Baseline SSH from attacker to server" width="45%">

**pfSense packet capture (attacker → server)**
<img src="https://i.imgur.com/P3JaWdy.png" alt="pfSense packet capture of attacker-to-server traffic" width="45%">

**Ping from attacker → workstation**
<img src="https://i.imgur.com/amkWHpx.png" alt="Baseline ping from attacker to internal workstation" width="45%">

**Failed curl/ssh from attacker → workstation (expected after rules)**
<img src="https://i.imgur.com/borAgQu.png" alt="Blocked curl and SSH from attacker to workstation after rules applied" width="45%">

**Ping & curl within external network**
<img src="https://i.imgur.com/9Ku8xDv.png" alt="Ping and curl within the external attacker network" width="45%">

**Internal workstation → internal server traffic**
<img src="https://i.imgur.com/aPPdxJM.png" alt="Traffic from internal workstation to internal server" width="45%">

### Nmap Baseline Examples

<img src="https://i.imgur.com/pMhYda2.png" alt="Nmap baseline scan results, part 1" width="60%">
<img src="https://i.imgur.com/NrFsGvb.png" alt="Nmap baseline scan results, part 2" width="60%">

---

## Security Policy Implementation

Policy enforcement was implemented primarily in pfSense, with host-level controls (iptables) filling gaps the router could not express.

**Corporate policy summary**

- Server: HTTP and SSH allowed internally; HTTP allowed externally (read-only).
- Workstations: may initiate internal access but may not host external services.
- Server must not initiate outbound external connections.
- External hosts must not be able to ping internal hosts.

### Access Control Matrix

<img src="https://i.imgur.com/p4HTJUe.png" alt="Access-control matrix mapping sources, destinations, and allowed protocols" width="60%">

### pfSense Rules (Visual Snapshots)

**WAN rules**
<img src="https://i.imgur.com/DOAkzbD.png" alt="pfSense WAN firewall rules" width="60%">

**LAN rules**
<img src="https://i.imgur.com/TYuD7lH.png" alt="pfSense LAN firewall rules" width="60%">

**OPT1 rules**
<img src="https://i.imgur.com/7YP6lje.png" alt="pfSense OPT1 firewall rules" width="60%">

---

## Post-Implementation Verification

- Nmap confirmed that only authorized services were exposed after the rules were applied.
- Wireshark captures verified that blocked traffic never traversed the internal interfaces.
- System and firewall logs corroborated rule enforcement.

<img src="https://i.imgur.com/bZeB44P.png" alt="Post-rule verification evidence, part 1" width="60%">
<img src="https://i.imgur.com/nYRv0ry.png" alt="Post-rule verification evidence, part 2" width="60%">
<img src="https://i.imgur.com/DthG8qW.png" alt="Post-rule verification evidence, part 3" width="60%">

---

## Host-Level Firewall (Server A.1)

Applied host-level iptables rules on the internal server to enforce policy items the gateway could not:

```bash
sudo iptables -A OUTPUT -d 192.168.20.0/24 -j DROP                       # block outbound to the external network
sudo iptables -A INPUT  -p tcp --dport 22 -s 192.168.10.0/24 -j ACCEPT   # allow SSH only from internal
sudo iptables -A INPUT  -p tcp --dport 80 -j ACCEPT                      # allow HTTP
sudo iptables -A INPUT  -p icmp -s 192.168.10.0/24 -j ACCEPT             # allow ICMP only from internal
```

<img src="https://i.imgur.com/f1mLB6a.png" alt="iptables ruleset on the internal server, part 1" width="60%"> <img src="https://i.imgur.com/W7tmMXL.png" alt="iptables ruleset on the internal server, part 2" width="60%">

## Part 1 Outcomes

- Built a reproducible lab environment for network-policy testing and security validation.
- Implemented defense-in-depth by combining router-level and host-level controls.
- Validated enforcement and produced reproducible evidence (scans, captures, and logs) to support detection and remediation workflows.

---

---

## Ethical Notice

All testing was performed in an authorized, isolated lab environment for defensive research only. Do not reproduce any of this against systems you do not own or have explicit permission to test.

## License

MIT — see [`LICENSE`](LICENSE). Copyright © 2025 Vansh Bhardwaj.
