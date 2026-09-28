# Slowloris Application-Layer DoS

> Part of the [Systems & Network Security Lab](README.md) — see the main README for the overview and the other tracks.

## Part 2 Overview

A controlled study of an application-layer Denial-of-Service attack (Slowloris) against an Apache2 web server, run inside the isolated sandbox from Part 1. The exercise centers on four things: attack mechanics, SIEM ingestion and detection, packet- and host-level telemetry analysis, and operational mitigations.

## Skills Demonstrated

- Application-layer DoS emulation (Slowloris)
- VirtualBox sandbox and pfSense topology management
- SIEM ingestion and detection engineering (Splunk)
- Network discovery and reconnaissance (Nmap)
- Packet- and host-level forensics (Wireshark, tcpdump, Apache logs)
- Reproducible documentation and defensive recommendations

## Why This Matters

Slowloris-style attacks can take a web service offline while using almost no bandwidth, which makes them easy to miss with network-volume monitoring alone. Executing *and* detecting the attack demonstrates the ability to reason about attacker technique, validate the telemetry that would catch it, and turn that into detection and response.

---

## Lab Environment & Topology

- **Network A (internal)** — `192.168.1.0/24`
  - Ubuntu (web server / Apache2): `192.168.1.10`
  - Windows XP (workstation): `192.168.1.15`
- **Network B (attacker / external)** — `192.168.2.0/24`
  - Kali Linux (attacker): `192.168.2.16`
- **Router / firewall:** pfSense (LAN ↔ Network A, OPT1/WAN ↔ Network B)
- **Monitoring:** Splunk, ingesting `/var/log/apache2/access.log` and `/var/log/apache2/error.log`

<img src="assets/Topology.png" alt="Slowloris lab network topology" width="600">

---

## Splunk Configuration & Ingestion Validation

Configured Splunk to ingest Apache logs from the victim VM:

- `/var/log/apache2/access.log` — sourcetype `apache:access`
- `/var/log/apache2/error.log` — sourcetype `apache:error`

**Validation steps**

1. Confirmed both log files appeared in Splunk with the expected sourcetypes.
2. Built an initial search to surface `MaxRequestWorkers` and related Apache errors.
3. Correlated Splunk event timestamps against live availability tests.

---

## Pre-Attack Assessment

From the attacker VM (Kali):

- Ran quick port/service enumeration with Nmap.
  <img src="assets/nmap-before.png" alt="Nmap scan of the target before the attack" width="600">

- Verified the HTTP service with curl:

  ```bash
  curl http://192.168.1.10/testsplunk
  ```

  <img src="assets/curl-before.png" alt="Successful curl response before the attack" width="425">

---

## Pre-Attack Tuning (Kali)

Slowloris holds open a large number of sockets, so the attacker's file-descriptor limits were raised first:

```bash
ulimit -S -n 8192    # soft limit
ulimit -H -n 16384   # hard limit
```

## Executing the Slowloris Attack

```bash
slowloris -v -p 80 -s 1500 --sleeptime 10 192.168.1.10
```

| Flag | Meaning |
|---|---|
| `-v` | Verbose output |
| `-p 80` | Target HTTP port |
| `-s 1500` | Number of sockets (tuned to the VM's limits) |
| `--sleeptime 10` | Seconds between header-refresh attempts |

<img src="assets/slowloris-attack.png" alt="Slowloris running against the target, opening and holding sockets" width="600">

## Evidence & Attack Verification

### Apache Error Log Evidence (Splunk)

Splunk surfaced worker-exhaustion indicators throughout the attack window.

<img src="assets/splunk-maxrequestworkers.png" alt="Splunk showing Apache MaxRequestWorkers errors during the attack" width="600">

### Application Availability Impact

Application-layer failure was confirmed with a cross-terminal test:

- HTTP requests stalled during the attack.
- ICMP (ping) traffic kept flowing.

This proves **application-layer resource exhaustion** rather than a full network outage — the host was still reachable, but Apache could no longer serve requests.

<img src="assets/ping-after.png" alt="Ping succeeding while HTTP stalls during the attack" width="500">

---

## Technical Analysis — Why the Attack Worked

Slowloris wins by exhausting application resources, not bandwidth.

**Attack mechanics**

- Opens many concurrent HTTP connections.
- Sends HTTP headers extremely slowly.
- Keeps connections alive with periodic header refreshes.
- Never completes a request, so Apache never releases the worker.

**Server-side behavior**

- Apache assigns a worker thread per connection.
- Workers stay locked waiting for headers that never finish arriving.
- Permissive keep-alive and timeout settings let those connections persist.
- Eventually `MaxRequestWorkers` is reached and legitimate clients are refused.

**Environmental factors that amplified it**

- Worker limits were low relative to the attacker's socket volume.
- Keep-alive and timeout settings were permissive.
- The attacker's socket capacity was raised via `ulimit` tuning.

---

## Detection Engineering Insights

Detection was validated across multiple independent signals:

**SIEM correlation**

- Apache error logs (`MaxRequestWorkers`).
- Availability metrics.
- Time-synchronized attack activity.

**Packet-level indicators**

- Partial or incomplete HTTP headers.
- Long-lived TCP sessions with no completed request.

---

## Security Impact

- Showed that a low-bandwidth application-layer attack can cause full service degradation.
- Reinforced the need to monitor application telemetry, not just network volume.
- Demonstrated why layered detection matters — the attack is invisible to bandwidth-only monitoring.

---

## Mitigation Recommendations

**Application hardening**

- Tune Apache worker and timeout thresholds.
- Enforce per-IP rate limiting.
- Restrict persistent connections where possible.

**Architecture defenses**

- Front the server with a reverse proxy or load balancer.
- Add upstream connection throttling.

**Detection improvements**

- Create SIEM alerts for worker-exhaustion patterns.
- Monitor for abnormal connection-persistence behavior.

---

## Part 2 Outcomes

- Executed and documented Slowloris DoS testing in an isolated environment.
- Built detection-validation workflows using SIEM and host/network telemetry.
- Produced defensive guidance backed by reproducible test evidence.

---

---

## Ethical Notice

All testing was performed in an authorized, isolated lab environment for defensive research only. Do not reproduce any of this against systems you do not own or have explicit permission to test.

## License

MIT — see [`LICENSE`](LICENSE). Copyright © 2025 Vansh Bhardwaj.
