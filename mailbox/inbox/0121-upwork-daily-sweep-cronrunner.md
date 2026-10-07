# 0121 — Daily 5 AM Upwork lead sweep via cronrunner (feeds 6 AM briefing)

expect-reply-within: 6h

## context

The Hatch-side morning briefing (6:00 AM MDT) sweeps Upwork/Freelancer for Stephen's remote-work leads. The browser-based Upwork sweep is unreliable: Cloudflare "Verify you are human" blocks detached runs with no user takeover possible (2026-10-07 produced zero verified Upwork leads — a coverage gap, not a quiet market).

The cronrunner profile has its own Upwork MCP token (OAuth completed 2026-10-06, read-only verified, 32/32 tools). The Upwork API won't hit browser bot challenges, so it's the robust path.

Goal: a 5:00 AM MDT daily job on the cronrunner profile that pulls fresh Upwork postings and drops them where the 6 AM briefing can read them from the repo.

## task

1. Using the cronrunner profile's Upwork MCP access — **read-only**: search/list jobs only. Never submit proposals, never message clients, never spend Connects. This is absolute.
2. Each morning at 5:00 AM America/Denver, run a job search with these terms (match the briefing's profile): ESP32, LoRa/LoRaWAN, MQTT, Home Assistant, embedded firmware, off-grid IoT, solar + LiFePO4/battery systems. Remote-only, part-time (<=30 hrs/week), US/EU clients preferred. Skip pure AutoCAD, utility-scale solar, and expert PCB-layout-only roles. Skip anything requiring identity verification Stephen hasn't done.
3. Window: postings from the last ~24h. Write ALL in-window leads (dedup against the briefing's seen list happens on the 6 AM side — include posting IDs so it can).
4. Write results to the repo at `handoff/upwork-leads/YYYY-MM-DD.md` (create the dir if missing), one file per day. Format per lead: posting ID, title, budget/rate, posted-ago, client country + brief history, one-line fit note, caveats (0-review client, non-English post, etc.). Header line with sweep timestamp and search terms used. If nothing is in-window, write the file anyway stating that explicitly.
5. Schedule it as a **standing daily** 5:00 AM America/Denver job on the cronrunner profile, using whatever scheduling the cronrunner profile uses. If the laptop is asleep/off at 5 AM, note in the reply what happens (catch-up run vs skipped).

## success_criteria

- A standing daily 5:00 AM MDT cronrunner job exists running the read-only Upwork sweep and writing `handoff/upwork-leads/<date>.md`.
- The first scheduled run's file (2026-10-08) is present in the repo with real lead data (or an explicit no-leads note) — written by the scheduled run, not by hand.
- Zero Upwork write operations performed.

## constraints

- Read-only on Upwork. No proposals, messages, Connects, profile edits. Ever.
- Use the cronrunner profile's token, NOT the DEFAULT profile.
- Keep it on the cheap model tier (Flash), off-peak.
- One deliverable: the scheduled sweep. Don't bundle other work.

## proof

Stage the reply with: the cronrunner schedule definition (what runs, when, which profile), the repo path + commit SHA of the first sweep file, and explicit confirmation that no Upwork write operations were performed.

## reply_format

Short summary: schedule confirmed (time, profile), first sweep file location, lead count in first run, laptop-asleep behavior, any blockers.
