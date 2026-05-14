#!/usr/bin/env python3
"""Render README.md to a print-friendly HTML next to it. Open in a browser and use Export/Print to PDF."""
from pathlib import Path
import markdown

REPO = Path(__file__).resolve().parents[1]
src = REPO / "README.md"
out = REPO / "Releases" / "README.html"

body = markdown.markdown(
    src.read_text(encoding="utf-8"),
    extensions=["extra", "sane_lists", "toc", "fenced_code", "tables"],
)

css = """
@page { size: Letter; margin: 0.75in; }
html { -webkit-print-color-adjust: exact; print-color-adjust: exact; }
body {
  font-family: -apple-system, "Helvetica Neue", Helvetica, Arial, sans-serif;
  font-size: 11pt;
  line-height: 1.5;
  color: #1a1a1a;
  max-width: 7in;
  margin: 0 auto;
  padding: 0.25in 0;
}
h1 { font-size: 24pt; margin: 0 0 0.2em; border-bottom: 2px solid #c9b87a; padding-bottom: 0.2em; }
h2 { font-size: 16pt; margin: 1.4em 0 0.4em; color: #4a3a1a; }
h3 { font-size: 12pt; margin: 1.2em 0 0.3em; color: #4a3a1a; }
h1, h2, h3 { page-break-after: avoid; }
p, ul, ol, table { page-break-inside: avoid; }
hr { border: 0; border-top: 1px solid #ddd; margin: 1.6em 0; }
a { color: #6b5320; text-decoration: none; }
code {
  font-family: "SF Mono", "Menlo", monospace;
  font-size: 0.9em;
  background: #f4efe2;
  padding: 0.1em 0.35em;
  border-radius: 3px;
}
pre {
  background: #f4efe2;
  padding: 0.8em 1em;
  border-radius: 4px;
  overflow-x: auto;
  font-size: 9.5pt;
  line-height: 1.4;
  page-break-inside: avoid;
}
pre code { background: transparent; padding: 0; }
blockquote {
  border-left: 3px solid #c9b87a;
  margin: 1em 0;
  padding: 0.2em 1em;
  color: #555;
  background: #faf7ee;
}
table { border-collapse: collapse; width: 100%; margin: 1em 0; }
th, td { border: 1px solid #ddd; padding: 0.4em 0.7em; text-align: left; }
th { background: #f4efe2; }
img { max-width: 100%; height: auto; }
ul, ol { padding-left: 1.4em; }
li { margin: 0.2em 0; }
"""

html = f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Robin Control Lite — README</title>
<style>{css}</style>
</head>
<body>
{body}
</body>
</html>
"""

out.write_text(html, encoding="utf-8")
print(f"wrote {out}")
