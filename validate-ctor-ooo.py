#!/usr/bin/env nix-shell
#! nix-shell -p python314 python314Packages.libclang -i python3
# check_ctor_dtor_steps.py
import clang.cindex
import sys
import re
from pathlib import Path

def extract_steps(cursor):
    steps = []
    if not cursor:
        return steps
    # Walk direct children of the compound statement (the body)
    for child in cursor.get_children():
        # Get attached or preceding /// comment
        comment = child.raw_comment or ""
        if not comment:
            # fallback: look at tokens immediately before the statement
            tokens = list(child.get_tokens())
            for tok in reversed(tokens[:5]):  # look back a few tokens
                if tok.kind == clang.cindex.TokenKind.COMMENT and "///" in tok.spelling:
                    comment = tok.spelling
                    break
        match = re.search(r"///\s*Step\s+(\d+):\s*(.+)", comment)
        if match:
            step_num = int(match.group(1))
            desc = match.group(2).strip()
            steps.append((step_num, desc, child))
    return steps

def check_file(filepath):
    index = clang.cindex.Index.create()
    tu = index.parse(str(filepath), args=["-std=c++20", "-fparse-all-comments"])
    for cursor in tu.cursor.get_children():
        if cursor.kind != clang.cindex.CursorKind.CXX_RECORD_DECL:
            continue
        # Find ctor and dtor in this class
        ctor, dtor = None, None
        for child in cursor.get_children():
            if child.kind == clang.cindex.CursorKind.CXX_CONSTRUCTOR_DECL:
                ctor = child
            elif child.kind == clang.cindex.CursorKind.CXX_DESTRUCTOR_DECL:
                dtor = child
        if not ctor or not dtor:
            continue

        ctor_steps = extract_steps(ctor.get_children()[-1] if ctor.get_children() else None)  # body
        dtor_steps = extract_steps(dtor.get_children()[-1] if dtor.get_children() else None)

        if not ctor_steps:
            continue

        # Check reverse order + [undo]
        expected = [(s[0], s[1]) for s in reversed(ctor_steps)]
        actual = [(s[0], s[1]) for s in dtor_steps]
        if actual != expected:
            print(f"ERROR in {filepath}:{cursor.location.line} class {cursor.spelling}")
            print(f"  Expected dtor steps (reverse): {expected}")
            print(f"  Got: {actual}")
            # You can make this raise or collect violations

if __name__ == "__main__":
    for f in sys.argv[1:]:
        check_file(Path(f))
