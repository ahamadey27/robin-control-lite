#!/usr/bin/env bash
# Build the Robin Control Lite manual PDF from its Markdown source.
# Requires: python3 with the `markdown` module, Google Chrome.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/Robin Control Lite Manual v1.0.1.md"
HTML="$ROOT/build/Robin Control Lite Manual v1.0.1.html"
PDF="$ROOT/Robin Control Lite Manual v1.0.1.pdf"
CHROME="/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"

mkdir -p "$ROOT/build"

# Markdown -> styled, standalone HTML
python3 - "$SRC" "$HTML" <<'PY'
import sys, markdown

src, out = sys.argv[1], sys.argv[2]
with open(src, encoding="utf-8") as f:
    body = markdown.markdown(f.read(), extensions=["tables", "sane_lists"])

CSS = """
@page { size: letter; margin: 22mm 20mm; }
* { box-sizing: border-box; }
body {
  font-family: -apple-system, "Helvetica Neue", Arial, sans-serif;
  font-size: 11.5pt; line-height: 1.5; color: #1a1a1a;
  max-width: 720px; margin: 0 auto;
}
h1 { font-size: 24pt; margin: 0 0 2pt; }
h2 { font-size: 15pt; margin: 24pt 0 6pt; padding-bottom: 3pt;
     border-bottom: 1px solid #ddd; }
h3 { font-size: 12pt; margin: 14pt 0 4pt; }
p { margin: 6pt 0; }
ul, ol { margin: 6pt 0; padding-left: 20pt; }
li { margin: 2pt 0; }
hr { border: 0; border-top: 1px solid #e5e5e5; margin: 16pt 0; }
code { font-family: "SF Mono", Menlo, monospace; font-size: 10pt;
       background: #f4f4f4; padding: 1px 4px; border-radius: 3px; }
table { border-collapse: collapse; width: 100%; margin: 8pt 0; font-size: 11pt; }
th, td { text-align: left; padding: 5pt 8pt; border-bottom: 1px solid #e5e5e5;
         vertical-align: top; }
th { border-bottom: 2px solid #ccc; }
em { color: #555; }
"""

html = f"""<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<title>Robin Control Lite Manual v1.0.1</title>
<style>{CSS}</style></head><body>
{body}
</body></html>"""

with open(out, "w", encoding="utf-8") as f:
    f.write(html)
print("wrote", out)
PY

# HTML -> PDF via headless Chrome
"$CHROME" --headless --disable-gpu --no-pdf-header-footer \
  --print-to-pdf="$PDF" "file://$HTML" 2>/dev/null

echo "PDF: $PDF"
