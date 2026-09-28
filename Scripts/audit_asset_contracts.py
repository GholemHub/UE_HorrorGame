"""Read-only source/package contract check. Run from any directory with Python 3."""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "Config" / "AssetContracts.json"
PACKAGE_RE = re.compile(rb"/Game/HorrorEngine/Audio/[A-Za-z0-9_/]+")


def package_file(path: str) -> Path:
    return ROOT / "Content" / (path.removeprefix("/Game/") + ".uasset")


def main() -> int:
    contract = json.loads(MANIFEST.read_text(encoding="utf-8"))
    required_missing = []
    for path in contract["required_game_assets"]:
        package = package_file(path)
        if not package.exists() and not package.with_suffix(".umap").exists():
            required_missing.append(path)

    structures = ROOT / "Content" / "HorrorEngine" / "Blueprints" / "Structures"
    missing_references: dict[str, list[str]] = {}
    for structure in sorted(structures.rglob("*.uasset")):
        for match in set(PACKAGE_RE.findall(structure.read_bytes())):
            package_path = match.decode("ascii")
            if not package_file(package_path).exists():
                missing_references.setdefault(package_path, []).append(
                    structure.relative_to(ROOT).as_posix()
                )

    known = set(contract["known_missing_legacy_audio"])
    new_missing = sorted(set(missing_references) - known)
    resolved = sorted(known - set(missing_references))
    report = {
        "schema_version": 1,
        "structures_scanned": len(list(structures.rglob("*.uasset"))),
        "required_missing": sorted(required_missing),
        "known_missing_legacy_audio": {
            path: missing_references[path]
            for path in sorted(set(missing_references) & known)
        },
        "new_missing_audio": new_missing,
        "resolved_since_baseline": resolved,
        "scope": "Serialized package path strings in legacy structures; not proof of live graph execution or cook inclusion."
    }
    output = ROOT / "Saved" / "Tests" / "Optimization" / "asset_contracts.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Required missing: {len(required_missing)}; known legacy audio missing: "
          f"{len(report['known_missing_legacy_audio'])}; new missing: {len(new_missing)}")
    print(output)
    return 1 if required_missing or new_missing else 0


if __name__ == "__main__":
    raise SystemExit(main())
