#!/usr/bin/env python3
"""Export the footprints used in the schematic to a standalone DipTrace pattern library.

Reads beeper.dchxml (the master schematic, edited in DipTrace), takes the patterns that its
placed parts use, and writes them with their pad styles to beeper_patterns.libxml. Open or
add that file in Pattern Editor, or attach it as a library in PCB Layout / Component Editor.

beeper_patterns.libxml has since been edited in DipTrace (3D models), so the script does not
overwrite an existing file unless --force is given.

Run:  python export_patterns.py [--force]
"""

import copy
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

HERE = Path(__file__).resolve().parent
SOURCE = HERE / "beeper.dchxml"
TARGET = HERE / "beeper_patterns.libxml"


def main():
    if TARGET.exists() and "--force" not in sys.argv:
        sys.exit(f"{TARGET.name} exists and may contain edits made in DipTrace; use --force to overwrite")
    root = ET.parse(SOURCE).getroot()
    lib = root.find("Library")
    plib = lib.find("Library")
    styles = {s.get("Name"): s for s in plib.find("PadStyles")}
    patterns = {p.get("PatternStyle"): p for p in plib.find("Patterns")}
    components = {c.get("ComponentStyle"): c.find("Part") for c in lib.find("Components")}

    # Patterns used by placed parts, ordered by the first RefDes that uses them.
    used = {}
    for part in root.find("Schematic/Components"):
        link = components[part.get("ComponentStyle")].find("Pattern")
        if link is not None:
            used.setdefault(link.get("Style"), []).append(part.findtext("RefDes"))
    order = sorted(used, key=lambda style: sorted(used[style])[0])

    out = ET.Element("Library", {
        "Type": "DipTrace-PatternLibrary", "Name": "Beeper", "Hint": "Padauk beeper footprints",
        "Version": plib.get("Version"), "UID32": "1587266402", "Units": plib.get("Units")})
    out_styles = ET.SubElement(out, "PadStyles")
    out_patterns = ET.SubElement(out, "Patterns")

    style_names = []
    seen = set()
    for style in order:
        pattern = copy.deepcopy(patterns[style])
        name = pattern.findtext("Name")
        if name in seen:            # same footprint used by several components
            continue
        seen.add(name)
        pattern.attrib.pop("PatternStyle", None)
        pattern.set("Id", str(len(out_patterns)))
        out_patterns.append(pattern)
        pads = [pad.get("Style") for pad in pattern.find("Pads")]
        default = pattern.find("DefPad")
        if default is not None:
            pads.append(default.get("Style"))
        for pad_style in pads:
            if pad_style not in style_names:
                style_names.append(pad_style)
        print(f"{pattern.get('Id')}: {name}  ({', '.join(sorted(used[style]))})")

    for name in style_names:
        out_styles.append(copy.deepcopy(styles[name]))

    ET.indent(out, space=" ")
    TARGET.write_text('<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(out, encoding="unicode") + "\n",
                      encoding="utf-8")
    print("wrote", TARGET.name)


if __name__ == "__main__":
    main()
