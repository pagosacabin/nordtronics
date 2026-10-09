---
task_id: "0134"
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0134 — Scope: Hermes on WhatsApp via Stephen's Google Voice number

Stephen's call (2026-10-09). Scoping only — do not build anything on this task.

## Context

- Stephen wants to message Hermes directly from WhatsApp, the way he messages Juno from WhatsApp today.
- Juno's WhatsApp side-chat (Muse's one-to-one provider connection, on Stephen's regular number) stays exactly as-is. This task does not touch it, reconfigure it, or split it.
- The new channel is a separate WhatsApp identity: Stephen's Google Voice number **(720) 837-1721**, which he is dedicating to Hermes. Stephen messages that number's WhatsApp account; Hermes responds as himself.
- Hermes runs on Stephen's own machine. Whatever serves this channel runs there.
- Juno's standing rules apply: never assume what your local installation has — only what you have proven on your own machine. Start by stating what you already know about running WhatsApp bots/clients (libraries, APIs, account and verification requirements) before doing any new research.

## Task

Write a scoping report (one deliverable) that answers whether this is worth building and what it would take. Save it as `docs/hermes-whatsapp-scope.md` on your branch.

The report must cover, at minimum:

1. **Account feasibility** — can a WhatsApp account be registered and verified on a Google Voice number (SMS and/or voice-call verification)? What are the known gotchas?
2. **Software options** — the realistic ways to run a WhatsApp client/bot on your machine (e.g. unofficial libraries, official Business API), with honest pros/cons of each for this use case.
3. **Bridge design** — how an incoming WhatsApp message would reach your agent runtime and how your reply would get back out. Sketch the moving parts, not code.
4. **Risks** — WhatsApp ToS / ban risk for unofficial clients, session expiry and re-pairing pain, what breaks when the machine reboots, any privacy exposure of Stephen's messages.
5. **Effort estimate** — your honest rough sizing (hours, not dates) for a minimal working version.
6. **Needs from Stephen** — anything he would have to provide or do (accounts, verification steps, approvals). Name exact items; never ask him for credentials in chat — anything secret gets staged at `/home/astroboy/.hermes/` per the standing convention.
7. **Recommendation** — build it, skip it, or a cheaper alternative that gets Stephen what he wants. Say which and why.

## Success criteria

- `docs/hermes-whatsapp-scope.md` exists on the task branch and covers all seven sections above with concrete findings, not placeholders.
- The recommendation in section 7 is a clear single call (build / skip / alternative) with reasoning.
- No accounts created, nothing registered, nothing installed, no verification codes requested.

## Constraints

- This task is read-only scoping: research and write. Create no accounts, register no numbers, install no packages, spend no money, and request nothing from Stephen.
- State cost constraints explicitly anywhere the report discusses build options (e.g. Business API per-message pricing vs free unofficial libraries).
- Keep the report honest about what you verified versus what you inferred — flag every inference as yours.

## Proof

- Branch on origin containing `docs/hermes-whatsapp-scope.md`; report the branch name and tip SHA.
- Move this file to `staged/` with `status: staged` and the proof pointers filled in.

## Reply format

When staging, fill in:

- `proof:` — branch name + tip SHA (list form per the mailbox README).
- `notes:` — your one-paragraph bottom line: the section-7 recommendation and the single biggest risk.
- Keep the full findings in the report file, not in `notes`.
