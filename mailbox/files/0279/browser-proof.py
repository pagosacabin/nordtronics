#!/usr/bin/env python3
"""Drive the LIVE page against the LIVE bridge, in a real browser.

This is the end-to-end proof for task 0279: the page's own Juno mode, its own
junoStart()/junoSendAndPoll() code, over its own HTTPS base URL, with the
browser's CORS enforcement in the loop. Nothing is stubbed.

    /usr/bin/python3 browser-proof.py [outdir]
"""
import asyncio
import json
import sys

from playwright.async_api import async_playwright

PAGE = "https://nordtronics.io/pocket-conquest/"
OUT = sys.argv[1] if len(sys.argv) > 1 else "."


async def main():
    async with async_playwright() as play:
        browser = await play.chromium.launch()
        context = await browser.new_context(viewport={"width": 1280, "height": 1100})
        # Chrome's Local Network Access check: nordtronics.io (public origin)
        # fetching home.tail2bff3f.ts.net (private address) needs the user's
        # one-time permission. Headless Chromium denies it silently, which is
        # what produced the page's "Failed to fetch" even with the API present.
        # Granting it here is the equivalent of the user clicking "Allow" on the
        # prompt a real browser shows for this origin.
        await context.grant_permissions(["local-network-access"], origin="https://nordtronics.io")
        page = await context.new_page()
        console = []
        page.on("console", lambda msg: console.append("%s: %s" % (msg.type, msg.text)))

        await page.goto(PAGE, wait_until="networkidle")
        title = await page.title()
        print("page title: %s" % title)

        # Switch "Play against" to Juno; the address box is pre-filled.
        await page.select_option("#modeSel", "juno")
        await page.wait_for_selector("#mode-juno:not(.hidden)")
        base = await page.input_value("#junoUrl")
        print("juno base url in the page: %s" % base)

        await page.click("#junoStartBtn")
        await page.wait_for_function(
            "() => /Juno game created/.test(document.getElementById('junoStatus').textContent)",
            timeout=30000,
        )
        print("after Start Juno game: %s"
              % (await page.text_content("#junoStatus")).strip())

        # Play one real turn through the UI: select the capital's stack, tap an
        # adjacent tile, then End turn (which POSTs the turn and polls).
        await page.click('[data-tile="B5"]')
        await page.wait_for_timeout(300)
        await page.click('[data-tile="B4"]')
        await page.wait_for_timeout(500)
        state_before = await page.evaluate("() => ({turn: S.turn, ply: S.ply, "
                                           "b4: S.tiles.B4.owner, armies: S.tiles.B4.armies})")
        print("local state after the player's move: %s" % json.dumps(state_before))

        await page.click("#endTurnBtn")
        await page.wait_for_function(
            "() => /Juno replied/.test(document.getElementById('junoStatus').textContent)",
            timeout=45000,
        )
        status = (await page.text_content("#junoStatus")).strip()
        state_after = await page.evaluate(
            "() => ({turn: S.turn, ply: S.ply, winner: S.winner,"
            " emberGold: S.gold.ember,"
            " emberTiles: Object.keys(S.tiles).filter(k => S.tiles[k].owner === 'ember')})")
        print("after End turn + poll: %s" % status)
        print("state after Juno's reply: %s" % json.dumps(state_after))
        print("any 'Failed to fetch' on the page: %s"
              % ("Failed to fetch" in (await page.content())))

        await page.screenshot(path="%s/browser-juno-mode.png" % OUT, full_page=True)
        print("screenshot: %s/browser-juno-mode.png" % OUT)
        if console:
            print("console messages:")
            for line in console:
                print("  " + line)
        await browser.close()
        return 0 if state_after.get("turn") == "nord" else 1


if __name__ == "__main__":
    raise SystemExit(asyncio.run(main()))
