# VPS Hardening Runbook — nordtronics.io

Target: Contabo Cloud VPS, Ubuntu 24.04 LTS. (Hetzner rejected the account
2026-09-24; Contabo Cloud VPS 4 in Seattle was purchased instead.)
Execute top to bottom the day the server is provisioned, before installing
any application services (Mosquitto, API, database). Assumes the Juno +
Stephen SSH keys were added at server creation time.

## 0. Prerequisites

- Server IP from the Contabo panel (my.contabo.com).
- SSH access as root with key: `ssh -i ~/.ssh/juno_hetzner root@<IP>`
- If the key wasn't installed at signup (Contabo only injects keys at
  deploy/reinstall time), log in once as root with the signup password and
  paste the key into `/root/.ssh/authorized_keys`, or use the panel's
  VNC console to do the same.
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
# deploy has no password, so sudo group membership alone leaves sudo
# unusable — grant passwordless sudo via a drop-in (validated with visudo):
printf 'deploy ALL=(ALL) NOPASSWD:ALL\n' > /etc/sudoers.d/deploy
chmod 440 /etc/sudoers.d/deploy && visudo -c
```

Verify: `ssh -i ~/.ssh/juno_hetzner deploy@<IP>` succeeds. All further
steps run as `deploy` (use sudo).

## 2. SSH hardening — `/etc/ssh/sshd_config.d/00-hardening.conf`

The file must sort BEFORE the cloud image's `50-cloud-init.conf`, which
ships `PasswordAuthentication yes` — sshd uses first-obtained-value-wins,
so a `99-` file would silently lose.

```
PasswordAuthentication no
ChallengeResponseAuthentication no
PermitRootLogin no
PermitEmptyPasswords no
X11Forwarding no
MaxAuthTries 3
```

```bash
sudo sshd -t && sudo systemctl reload ssh   # service is ssh.service, not sshd.service
sudo sshd -T | grep -iE '^(passwordauthentication|permitrootlogin)'  # must show "no" for both
```

Keep the original root terminal open while testing a new SSH session in
another window. Only close it after the new session connects.

## 3. Automatic security updates

```bash
# Preseed debconf answers first — without a tty, apt-listchanges blocks
# on an interactive prompt and the install hangs.
echo "unattended-upgrades unattended-upgrades/enable_auto_updates boolean true" | sudo debconf-set-selections
echo "apt-listchanges apt-listchanges/which select news" | sudo debconf-set-selections
echo "apt-listchanges apt-listchanges/frontend select text" | sudo debconf-set-selections
echo "apt-listchanges apt-listchanges/email-address string root" | sudo debconf-set-selections
echo "apt-listchanges apt-listchanges/save-seen boolean true" | sudo debconf-set-selections
echo "apt-listchanges apt-listchanges/confirm boolean false" | sudo debconf-set-selections
sudo apt install -y unattended-upgrades apt-listchanges < /dev/null
sudo dpkg-reconfigure -f noninteractive -plow unattended-upgrades < /dev/null
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

Also enable the Contabo panel firewall with the same port set, so there
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

- Enable Contabo automated backups / take a manual snapshot in the panel.
- Before any risky change, take a manual snapshot in the panel.

## Verification checklist

- [ ] `ssh deploy@<IP>` works with key; password login rejected
- [ ] `ssh root@<IP>` rejected
- [ ] `sudo ufw status` shows deny-incoming with only 22/80/443/8883 open
- [ ] Contabo panel firewall mirrors the same rules
- [ ] `fail2ban-client status sshd` shows the jail active
- [ ] unattended-upgrades enabled
- [ ] DNS resolves for `@`, `mqtt`, `api`
- [ ] `certbot certificates` shows valid certs covering all three names
- [ ] No service running as root: `ps -U root -u root -N` reviewed

Only after every box is checked: install Mosquitto, the ingest worker,
time-series DB, and API — each under its own service user.
