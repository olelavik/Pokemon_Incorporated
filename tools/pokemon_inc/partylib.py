"""Pokémon Incorporated: shared helper for rewriting trainer parties.

set_party(trainers_src, parties_src, 'TRAINER_X', mons) replaces the trainer's
party array in trainer_parties.h and its .party macro in trainers.h.

A mon is a tuple (species, level) or a dict with keys
species, lvl, item (optional), moves (optional list of 4), iv (optional).
The smallest struct that fits is used: default moves without items when
possible, item + custom moves otherwise.
"""
import re


def _norm(mon):
    if isinstance(mon, tuple):
        mon = {'species': mon[0], 'lvl': mon[1]}
    mon.setdefault('item', None)
    mon.setdefault('moves', None)
    mon.setdefault('iv', None)
    return mon


def party_array(name, mons, default_iv):
    mons = [_norm(m) for m in mons]
    has_item = any(m['item'] for m in mons)
    has_moves = any(m['moves'] for m in mons)
    if has_moves:
        assert all(m['moves'] for m in mons), name + ': custom moves need all mons to have them'
    struct, macro = {
        (False, False): ('TrainerMonNoItemDefaultMoves', 'NO_ITEM_DEFAULT_MOVES'),
        (False, True): ('TrainerMonNoItemCustomMoves', 'NO_ITEM_CUSTOM_MOVES'),
        (True, False): ('TrainerMonItemDefaultMoves', 'ITEM_DEFAULT_MOVES'),
        (True, True): ('TrainerMonItemCustomMoves', 'ITEM_CUSTOM_MOVES'),
    }[(has_item, has_moves)]
    out = [f'static const struct {struct} {name}[] = {{']
    for m in mons:
        out.append('    {')
        out.append(f'        .iv = {m["iv"] if m["iv"] is not None else default_iv},')
        out.append(f'        .lvl = {m["lvl"]},')
        out.append(f'        .species = SPECIES_{m["species"]},')
        if has_item:
            out.append(f'        .heldItem = ITEM_{m["item"] or "NONE"},')
        if has_moves:
            out.append('        .moves = {' + ', '.join('MOVE_' + x for x in m['moves']) + '},')
        out.append('    },')
    out.append('};')
    return '\n'.join(out), macro


def get_party_name(trainers_src, trainer):
    m = re.search(r'\[' + trainer + r'\] = \{.*?\.party = \w+\((\w+)\)', trainers_src, re.S)
    return m.group(1)


def get_party(trainers_src, parties_src, trainer):
    """[(species, lvl, iv)] of the trainer's current party."""
    name = get_party_name(trainers_src, trainer)
    arr = re.search(r'struct \w+ ' + name + r'\[\] = \{(.*?)\n\};', parties_src, re.S).group(1)
    return [(s, int(l), int(iv)) for iv, l, s in
            re.findall(r'\.iv = (\d+),\s*\.lvl = (\d+),\s*\.species = SPECIES_(\w+)', arr)]


def set_party(trainers_src, parties_src, trainer, mons, default_iv=0):
    name = get_party_name(trainers_src, trainer)
    text, macro = party_array(name, mons, default_iv)
    pat = re.compile(r'static const struct TrainerMon\w+ ' + name + r'\[\] = \{.*?\n\};', re.S)
    parties_src, n = pat.subn(lambda _: text, parties_src, count=1)
    assert n == 1, trainer
    entry = re.compile(r'(\[' + trainer + r'\] = \{.*?\.party = )\w+\(' + name + r'\)', re.S)
    trainers_src, n = entry.subn(lambda m: m.group(1) + f'{macro}({name})', trainers_src, count=1)
    assert n == 1, trainer
    return trainers_src, parties_src


def set_field(trainers_src, trainer, field, value):
    entry = re.compile(r'(\[' + trainer + r'\] = \{.*?\.' + field + r' = )[^\n]*?(,\n)', re.S)
    trainers_src, n = entry.subn(lambda m: m.group(1) + value + m.group(2), trainers_src, count=1)
    assert n == 1, (trainer, field)
    return trainers_src
