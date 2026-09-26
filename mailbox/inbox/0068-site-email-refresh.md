---
task_id: "0068"
status: inbox
iteration: 0
---

# 0068 — Website: replace placeholder email and refresh stale content

## Context

The full site (task 0051) is live at https://nordtronics.io, deployed from `main`
at SHA `a79383dfc4ac4d6385724f4a6f6554e9948b4f75` (green CI run 36025328226).
The site source still contains the placeholder address `hello@example.com` in one
or more places. The real contact address is `hello@nordtronics.io`, which is
already live: it routes free through Cloudflare Email Routing to Stephen's Gmail,
and replies from his Gmail work (verified end to end).

## Task

One deliverable: replace every instance of `hello@example.com` (and any other
placeholder contact address, phone, or dummy link text) in the site source with
`hello@nordtronics.io`, and fix anything else you find along the way that is
obviously stale, broken, or wrong (dead links, outdated copy, wrong year,
lorem ipsum, etc.). Keep it conservative: no redesign, no new pages, no
content sections — only corrections to what exists.

## Success criteria

- `grep -ri "example\.com" <site source>` returns zero hits (excluding
  node_modules / vendored deps).
- Every `mailto:` link and visible contact address on the built site points to
  `hello@nordtronics.io`.
- CI builds green on the merge, and https://nordtronics.io serves the updated
  site afterward.

## Constraints

- Work in a branch named `hermes/0068-site-email-refresh`; merge via PR into
  `main` so CI runs on the merge. Deploy happens from `main` only.
- One deliverable per task: corrections only, no feature work. If you find
  something that wants a bigger change, list it in the reply notes instead of
  building it.
- Run this task on the deepseek-flash worker tier. State the model used in your
  reply.
- Never touch `handoff/` paths, the mailbox protocol, or anything outside the
  site source for this task.

## Proof

- `proof` front-matter: branch `hermes/0068-site-email-refresh` + tip SHA, and
  the green Actions run URL whose head SHA matches that tip.
- I will independently fetch https://nordtronics.io and check the contact
  address renders as `hello@nordtronics.io`.

## Reply format

Stage the file with `status: staged` and fill in:

```yaml
proof:
  - branch: hermes/0068-site-email-refresh
    sha: <40-char tip sha>
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/<id>
```

Plus in the body or notes:
- List of files changed and what changed in each.
- `grep` evidence showing zero remaining `example.com` hits in the site source.
- Model/tier used.
