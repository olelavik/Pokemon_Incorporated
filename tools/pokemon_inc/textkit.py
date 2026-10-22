"""Helpers for rewriting pokefirered .inc dialogue by label, with pixel-width checks."""
import re, os

ROOT = os.path.expanduser('~/pokefirered')
_cm = None
_widths = None
MAX_W = 196  # widest line in vanilla FireRed dialogue

def _load():
    global _cm, _widths
    if _cm is not None:
        return
    _cm = {}
    for line in open(os.path.join(ROOT, 'charmap.txt'), encoding='utf-8'):
        line = line.split('@')[0].strip()
        if line.startswith("'") and '=' in line:
            k, v = line.rsplit('=', 1)
            k = k.strip()[1:-1]
            v = v.split()
            if len(v) == 1 and len(k) == 1:
                _cm.setdefault(k, int(v[0], 16))
    _cm["'"] = 0xB4
    src = open(os.path.join(ROOT, 'src/text.c')).read()
    body = src.split('sFontNormalLatinGlyphWidths[] =', 1)[1].split('};', 1)[0]
    _widths = [int(x, 0) for x in re.findall(r'0x[0-9A-Fa-f]+|\b\d+\b', body)]

PLACEHOLDER_W = {'{PLAYER}': 42, '{RIVAL}': 42, '{STR_VAR_1}': 60, '{STR_VAR_2}': 42, '{STR_VAR_3}': 42}

def width(s):
    _load()
    w = 0
    for ph, pw in PLACEHOLDER_W.items():
        while ph in s:
            s = s.replace(ph, '', 1)
            w += pw
    s = re.sub(r'\{[A-Z_0-9 ]+\}', '', s)
    for c in s:
        code = _cm.get(c, 0)
        w += _widths[code] if code < len(_widths) else 6
    return w

def to_strings(text, indent='    '):
    """text uses \\n \\l \\p markers and ends with $ ."""
    parts = re.split(r'(\\[nlp])', text)
    out, cur = [], ''
    for p in parts:
        cur += p
        if p in ('\\n', '\\l', '\\p'):
            out.append(cur); cur = ''
    if cur:
        out.append(cur)
    return ''.join(f'{indent}.string "{o}"\n' for o in out)

PROBLEMS = []

def check(label, text):
    for line in re.split(r'\\[nlp]', text.rstrip('$')):
        w = width(line)
        if w > MAX_W:
            PROBLEMS.append((label, w, line))

def set_text(path, label, text, indent=None):
    full = os.path.join(ROOT, path)
    s = open(full, encoding='utf-8').read()
    pat = re.compile(r'^(' + re.escape(label) + r'::\n)((?:[ \t]*\.string[^\n]*\n)+)', re.M)
    m = pat.search(s)
    if not m:
        raise KeyError(f'{label} not found in {path}')
    if indent is None:
        indent = re.match(r'[ \t]*', m.group(2)).group(0)
    check(label, text)
    s = s[:m.start(2)] + to_strings(text, indent) + s[m.end(2):]
    open(full, 'w', encoding='utf-8').write(s)

def add_text(path, label, text, indent='    '):
    full = os.path.join(ROOT, path)
    s = open(full, encoding='utf-8').read()
    if re.search(r'^' + re.escape(label) + r'::', s, re.M):
        return set_text(path, label, text)
    check(label, text)
    if not s.endswith('\n'):
        s += '\n'
    s += f'\n{label}::\n' + to_strings(text, indent)
    open(full, 'w', encoding='utf-8').write(s)

def report():
    if PROBLEMS:
        print('TOO WIDE:')
        for p in PROBLEMS:
            print('  ', p)
    else:
        print('all lines fit')
