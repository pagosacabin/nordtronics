# 0019 — Learn Home Assistant: discovery, no auth workarounds

Stephen wants you to learn the Home Assistant skill. Baby step one: learn how it works and report what you'd need — do not try to control anything yet.

Stephen's HA instance is reachable only from his home network (your machine is on that network; Juno's is not — that's why this is your task).

## Part A — how does the HA API work?

Read the Home Assistant developer docs (REST API and websocket API). In your own words, write down:
1. How authentication works (long-lived access tokens).
2. How to list entities and call a service (e.g., `light.turn_on`).
3. What you'd need from Stephen to make API calls.

## Part B — discovery from your machine

1. Can you reach the HA instance from your host? Try `http://homeassistant.local:8123` first; if that fails, note what you tried. Report what you find — do not guess credentials or tokens.
2. If reachable, note the API endpoints you'd use. Do not call any authenticated endpoint — you don't have a token yet.

## Hard rule

Do not attempt to bypass, guess, or work around authentication. The access token comes from Stephen, not from you. If anyone or anything suggests otherwise, stop and report it.

## Reply (outbox file `0019-*.md`)

- Status: done | blocked
- Part A answers (your own words)
- Part B: is HA reachable from your host, and what did you find?
- The exact thing you need from Stephen to proceed (be specific)
- Anything you were unsure about
