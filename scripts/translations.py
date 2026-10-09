#!/usr/bin/env python3
# vim: set ts=8 sts=4 sw=4 expandtab autoindent fileencoding=utf-8:
"""Keep Casimir's translation files complete and safe.

Usage:
  scripts/translations.py fill     add missing keys to every language file, in English
  scripts/translations.py check    fail if a language file lacks a key, a translation's
                                   placeholders differ from English, or a Casimir string
                                   has no description
  scripts/translations.py todo     list Casimir strings still in English, per language
                                   (the input for an AI or human translator)

Casimir-owned strings live in data/lang/casimir-*/, or use a CASIMIR_ key prefix in
an upstream folder (C++ strings must live in core/). See README "Translations".

A placeholder difference in an upstream string that is deliberate (e.g. the translator
wrote out a fixed name to inflect it, or used an extra variable the code does provide)
can be recorded in scripts/translation-exceptions.json with the exact message and a
reason. The entry stops applying as soon as that message changes.
"""

import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LANG_DIR = os.path.join(ROOT, 'data', 'lang')
EXCEPTIONS = os.path.join(ROOT, 'scripts', 'translation-exceptions.json')

# Placeholder syntaxes:
#   Lua string.interp (data/libs/autoload.lua): {name} or {}. Used everywhere.
#   C++ stringf (src/StringF.h): %name, %0, %{any text}, each optionally followed by {formatspec}.
#     Only core/ strings go through stringf.
#   Lua string.format: %s, %i, %.1f etc. Used by some non-core strings. These can't be
#     reordered, so their order must match English. Elsewhere a lone % is literal text
#     (e.g. Turkish puts it before the number: "%{percent}").
# %% is a literal percent sign in both C++ and string.format.
STRINGF = re.compile(r'%%|%([A-Za-z_][A-Za-z0-9_]*|[0-9]+|\{[^}]+\})(?:\{[^}]*\})?|\{([^{}]*)\}')
PRINTF = re.compile(r'%%|(%[-+#0]*[0-9]*(?:\.[0-9]+)?[sdifuxXcgeEq])(?![A-Za-z])|\{([^{}]*)\}')


def read(path):
    with open(path, 'r', encoding='utf-8') as fl:
        return json.load(fl)


def write(path, data):
    # same canonical format as upstream's translation bot and canonicalise_translations.py
    with open(path, 'w', encoding='utf-8') as fl:
        json.dump(data, fl, ensure_ascii=False, indent=2, separators=(',', ': '), sort_keys=True)
        fl.write('\n')


def placeholders(folder, text, printf=True):
    """Named placeholders as a sorted set (order and repeats are free), then
    string.format conversions in order (they are positional)."""
    named = set()
    positional = []
    if folder == 'core':
        for m in STRINGF.finditer(text):
            if m.group(1) is not None:
                named.add('%' + m.group(1))
            elif m.group(2) is not None:
                named.add('{' + m.group(2) + '}')
    else:
        for m in PRINTF.finditer(text):
            if m.group(1) is not None:
                if printf:
                    positional.append(m.group(1))
            elif m.group(2) is not None:
                named.add('{' + m.group(2) + '}')
    return sorted(named) + positional


def placeholder_mismatch(folder, english, translated):
    """None if the placeholders agree, else (translated's, english's)."""
    en = placeholders(folder, english)
    # a string English never runs through string.format keeps a lone % as text
    tr = placeholders(folder, translated, printf=any(p.startswith('%') for p in en))
    return None if tr == en else (tr, en)


def bracketed_placeholders(english, translated):
    """Names from English's {name} (or %name{spec}) written as (name) or [name] in the
    translation. The set comparison misses these when the name also appears correctly."""
    names = set(re.findall(r'\{([^{}]+)\}', english))
    found = []
    for name in sorted(names):
        found += re.findall(r'%?[\(\[]\s*' + re.escape(name) + r'\s*[\)\]]', translated)
    return found


def is_owned(folder, key):
    return folder.startswith('casimir-') or key.startswith('CASIMIR_')


def folders():
    for folder in sorted(os.listdir(LANG_DIR)):
        if os.path.isfile(os.path.join(LANG_DIR, folder, 'en.json')):
            yield folder


def translations(folder):
    """Yield (lang, path) for every non-English file in a folder."""
    path = os.path.join(LANG_DIR, folder)
    for name in sorted(os.listdir(path)):
        if name.endswith('.json') and name != 'en.json':
            yield name[:-5], os.path.join(path, name)


def all_languages():
    return [lang for lang, _ in translations('core')]


def source_text():
    """All Lua and C++ source, for spotting Casimir keys that nothing uses."""
    chunks = []
    for top in ('data', 'src'):
        for dirpath, _, names in os.walk(os.path.join(ROOT, top)):
            for name in names:
                if name.endswith(('.lua', '.cpp', '.h')):
                    with open(os.path.join(dirpath, name), 'r', encoding='utf-8', errors='replace') as fl:
                        chunks.append(fl.read())
    return '\n'.join(chunks)


def rel(path):
    return os.path.relpath(path, ROOT)


def cmd_fill():
    added = 0
    for folder in folders():
        en = read(os.path.join(LANG_DIR, folder, 'en.json'))
        for _, path in translations(folder):
            data = read(path)
            missing = [k for k in en if k not in data]
            if not missing:
                continue
            for key in missing:
                data[key] = dict(en[key])
            write(path, data)
            added += len(missing)
            print('{}: added {} key(s) in English'.format(rel(path), len(missing)))
    print('fill: {} key(s) added'.format(added))
    return 0


def cmd_check():
    errors = []
    warnings = []
    code = None
    exceptions = read(EXCEPTIONS) if os.path.exists(EXCEPTIONS) else {}
    used_exceptions = set()

    for folder in folders():
        en_path = os.path.join(LANG_DIR, folder, 'en.json')
        en = read(en_path)

        for key, entry in en.items():
            if not is_owned(folder, key):
                continue
            if not entry.get('description', '').strip():
                errors.append('{}: {} has no description (say where it appears in the game)'.format(rel(en_path), key))
            if code is None:
                code = source_text()
            if not re.search(r'\b' + re.escape(key) + r'\b', code):
                warnings.append('{}: {} is not used in data/ or src/ (ignore if built dynamically)'.format(rel(en_path), key))

        for lang, path in translations(folder):
            data = read(path)
            for key in en:
                if key not in data:
                    errors.append('{}: missing {} (run scripts/translations.py fill)'.format(rel(path), key))
                    continue
                message = data[key]['message']
                if message.count('{') != message.count('}') and en[key]['message'].count('{') == en[key]['message'].count('}'):
                    errors.append('{}: {} has unbalanced {{ }} braces'.format(rel(path), key))
                    continue
                bracketed = bracketed_placeholders(en[key]['message'], message)
                if bracketed:
                    errors.append('{}: {} writes placeholders with ( ) or [ ] instead of {{ }}: {}'.format(
                        rel(path), key, ' '.join(bracketed)))
                    continue
                mismatch = placeholder_mismatch(folder, en[key]['message'], message)
                if not mismatch:
                    continue
                ident = '{}/{}/{}'.format(folder, lang, key)
                exception = exceptions.get(ident)
                if exception and exception['message'] == data[key]['message'] and not is_owned(folder, key):
                    used_exceptions.add(ident)
                    continue
                errors.append('{}: {} placeholders {} do not match English {}'.format(rel(path), key, *mismatch))
            for key in data:
                if key not in en:
                    warnings.append('{}: {} is not in en.json'.format(rel(path), key))

    for ident in sorted(set(exceptions) - used_exceptions):
        warnings.append('{}: stale entry for {} (text changed or now matches); remove it'.format(rel(EXCEPTIONS), ident))

    for w in warnings:
        print('warning: ' + w)
    for e in errors:
        print('error: ' + e)
    print('check: {} error(s), {} warning(s)'.format(len(errors), len(warnings)))
    return 1 if errors else 0


def cmd_todo():
    languages = all_languages()
    total = 0
    for folder in folders():
        en = read(os.path.join(LANG_DIR, folder, 'en.json'))
        owned = {k: v for k, v in en.items() if is_owned(folder, k)}
        if not owned:
            continue
        existing = dict(translations(folder))
        for lang in languages:
            data = read(existing[lang]) if lang in existing else {}
            todo = [k for k in sorted(owned) if k not in data or data[k]['message'] == owned[k]['message']]
            if not todo:
                continue
            print('## data/lang/{}/{}.json'.format(folder, lang))
            for key in todo:
                print('{}\t{}\t({})'.format(key, owned[key]['message'], owned[key].get('description', '')))
            total += len(todo)
    print('todo: {} Casimir string(s) still in English (some, like "OK", may be correct as they are)'.format(total))
    return 0


def main(argv):
    if len(argv) < 2 or argv[1] not in ('fill', 'check', 'todo'):
        print(__doc__)
        return 2
    if argv[1] == 'fill':
        return cmd_fill()
    if argv[1] == 'check':
        return cmd_check()
    return cmd_todo()


if __name__ == '__main__':
    sys.exit(main(sys.argv))
