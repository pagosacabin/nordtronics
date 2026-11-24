# Librarian report — 2026-11-23

Date: 2026-11-23 · Window: after vault HEAD `600b0ab` (2026-11-23 21:41 MST) → 21:50 MST · Closed in window: **0250** (DAC calibration, verified/archived by Juno at 21:46 MST) · Pages: `bench/living-frame.md` updated, `INDEX.md` updated, `log/2026-11-23.md` extended · Vault HEAD: **`31c6769`**

Note on the Nov 18 → today catch-up: it was performed by Stephen's interactive session at
21:41 MST (commits `a9f6e81`…`600b0ab`, four pages + log), before this task was picked up. This
run therefore covered only the window after that commit — which turned out to hold exactly one
closure, 0250, whose archive commit landed at 21:46 MST while this run was in progress. Folded
into the bench page: what the measured ladder does to the emitted drive in γ-space, the
verification evidence (branch `7563ca0`, byte-identical rebuild, board digest matched, live shot
1280 pkts / 4162 ms / 0 fail, library 38 → 39), the fact that the voltage table is one reading
per cell, and the two things still open — the repeatability meter pass and the panel photos —
plus the frame board's SD slot failing physically after the measurement.

Correction, stated plainly: the window was first read as empty (the mailbox check ran before
Juno's archive commit landed) and a report saying so was staged at 21:47 MST in commit
`8454196`. It was then updated in place rather than a second report being added.

Recurring job: profile `cronrunner`, id `fd5302421a30`, name `crew-vault-librarian`, schedule
`0 17 * * 0` (Sundays 17:00 America/Denver), workdir `/home/astroboy/crew-vault`. It stages a
report of this shape each week at `mailbox/staged/librarian-report-YYYY-MM-DD.md`.
