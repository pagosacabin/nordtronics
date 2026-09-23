# VPS Hardening Runbook — nordtronics.io

Target: Hetzner VPS, Ubuntu 24.04 LTS. Execute top to bottom the day the
server is provisioned, before installing any application services
(Mosquitto, API, database). Assumes the Juno + Stephen SSH keys were added
at server creation time.

## 0. Prerequisites

- Server IP from Hetzner console.
- SSH access as root with key: `ssh -i ~/.ssh/juno_hetzner root@<IP>`
- Do not proceed past step 3 until key-based login as the admin user works.

## 1. First login — updates and admin user

```bash
apt update && apt upgrade -y
adduser --disabled-password --gecos "" deploy
usermod -aG sudo deploy
mkdir -p /home/deploy/.ssh
# paste the Juno + Stephen public keys into authorized_keys, one per line
chmod 700 /home/deploy/.ssh && chmod 600 /home/deploy/.ssh/authorized_keys
chown -R deploy:deploy /home/deploy/.ssh
```

Verify: `ssh -i ~/.ssh/juno_hetzner deploy@<IP>` succeeds. All further
steps run as `deploy` (use sudo).

## 2. SSH hardening — `/etc/ssh/sshd_config.d/99-hardening.conf`

```
PasswordAuthentication no
ChallengeResponseAuthentication no
PermitRootLogin no
PermitEmptyPasswords no
X11Forwarding no
MaxAuthTries 3
```

```bash
sudo systemctl reload sshd
```

Keep the original root terminal open while testing a new SSH session in
another window. Only close it after the new session connects.

## 3. Automatic security updates

```bash
sudo apt install -y unattended-upgrades apt-listchanges
sudo dpkg-reconfigure -plow unattended-upgrades   # answer Yes
```

Verify `/etc/apt/apt.conf.d/20auto-upgrades` contains:

```
APT::Periodic::Update-Package-Lists "1";
APT::Periodic::Unattended-Upgrade "1";
```

## 4. Host firewall — ufw

Default-deny inbound, allow only what the stack needs:

```bash
sudo apt install -y ufw
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow 22/tcp        # SSH
sudo ufw allow 80/tcp        # HTTP (ACME challenges)
sudo ufw allow 443/tcp       # HTTPS API
sudo ufw allow 8883/tcp      # MQTT over TLS (base stations)
# NOTE: plain MQTT 1883 stays closed — TLS only, even on day one
sudo ufw --force enable
sudo ufw status verbose
```

Also enable the Hetzner cloud firewall with the same port set, so there
are two independent layers.

## 5. fail2ban

```bash
sudo apt install -y fail2ban
sudo systemctl enable --now fail2ban
```

Defaults are fine for sshd on 24.04. Check: `sudo fail2ban-client status sshd`.

## 6. Time sync

`systemd-timesyncd` is enabled by default on 24.04. Confirm:

```bash
timedatectl show-timesync --all | grep -i "server\|ntp"
```

Accurate clocks matter for TLS cert validation and MQTT message timestamps.

## 7. Service users (least privilege)

Create one non-login user per service as they are installed. Pattern:

```bash
sudo adduser --system --no-create-home --disabled-login --group <svc>
```

Planned: `mosquitto` (created by the package), one for the ingest worker,
one for the API. No service ever runs as root or as `deploy`.

## 8. DNS

At the registrar (once `nordtronics.io` is owned), add:

- `A` record: `@` → `<server IPv4>`
- `AAAA` record: `@` → `<server IPv6>` (if assigned)
- `A`/`AAAA`: `mqtt`, `api` → same addresses

Wait for propagation (`dig +short nordtronics.io`) before step 9.

## 9. TLS — Let's Encrypt

```bash
sudo apt install -y certbot
sudo certbot certonly --standalone -d nordtronics.io -d mqtt.nordtronics.io -d api.nordtronics.io
```

Certificates land in `/etc/letsencrypt/live/`. Services read them from
there; add a deploy hook or cron to reload services on renewal
(`certbot renew` runs twice daily via systemd timer by default).

## 10. Backups

- Enable Hetzner automated backups in the console (weekly, retained by policy).
- Before any risky change, take a manual snapshot in the console.

## Verification checklist

- [ ] `ssh deploy@<IP>` works with key; password login rejected
- [ ] `ssh root@<IP>` rejected
- [ ] `sudo ufw status` shows deny-incoming with only 22/80/443/8883 open
- [ ] Hetzner cloud firewall mirrors the same rules
- [ ] `fail2ban-client status sshd` shows the jail active
- [ ] unattended-upgrades enabled
- [ ] DNS resolves for `@`, `mqtt`, `api`
- [ ] `certbot certificates` shows valid certs covering all three names
- [ ] No service running as root: `ps -U root -u root -N` reviewed

Only after every box is checked: install Mosquitto, the ingest worker,
time-series DB, and API — each under its own service user.
