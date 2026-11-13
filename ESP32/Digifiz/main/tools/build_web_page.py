"""Build the variant-specific embedded page and its deterministic gzip asset."""

import argparse
import gzip
from pathlib import Path
import re


def build_page(source: str, variant: str) -> str:
    begin = "<!-- REFIZ_ONLY_BEGIN -->"
    end = "<!-- REFIZ_ONLY_END -->"
    if source.count(begin) != source.count(end):
        raise ValueError("Unbalanced ReFIZ-only page markers")
    if variant == "next":
        return re.sub(re.escape(begin) + r".*?" + re.escape(end), "", source, flags=re.S)
    if variant == "refiz":
        return source.replace(begin, "").replace(end, "")
    raise ValueError(f"Unknown display variant: {variant}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--variant", choices=("next", "refiz"), required=True)
    args = parser.parse_args()
    data = build_page(args.source.read_text(encoding="utf-8"), args.variant).encode("utf-8")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(data)
    Path(str(args.output) + ".gz").write_bytes(gzip.compress(data, compresslevel=9, mtime=0))
