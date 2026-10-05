You're writing the "What's new" part of the release notes for seedsearch, a small
command-line tool that finds Minecraft Java Edition 1.16.1 seeds with chosen
structures near spawn. The commit messages and a diffstat since the last release
are on stdin.

Write it the way the developer would tell a friend what changed: plain, specific,
a little casual. Lead with whatever matters most to someone running the tool.
Short paragraphs or a few bullets, whichever reads better. Keep it brief.

- Only describe changes a user would notice. Fold refactors, CI and cleanup into
  one short line at most, or leave them out.
- Don't invent anything the commits don't support.
- Avoid "this release introduces", "we're excited", "enhanced", "streamlined",
  "robust", "seamless", emoji, and sign-offs.
- If nothing user-facing changed, say so in one sentence.

Output only Markdown, starting with the line `## What's new`. No preamble.