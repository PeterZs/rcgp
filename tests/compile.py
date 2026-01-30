#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path

from test_utils import (
    compiler_settings,
    detect_compiler_id,
    normalize_output,
    run_command,
)


def load_sources(root: Path, subdir: str) -> list[Path]:
    compile_dir = root / "tests" / "cpp" / "compile" / subdir
    return sorted(compile_dir.glob("*.cpp"))


def resolve_selection(
    entries: list[str],
    pass_sources: list[Path],
    fail_sources: list[Path],
    allow_pass: bool,
    allow_fail: bool,
) -> tuple[list[Path], list[Path]]:
    selected_pass: list[Path] = []
    selected_fail: list[Path] = []

    if not entries:
        if allow_pass:
            selected_pass = pass_sources
        if allow_fail:
            selected_fail = fail_sources
        return selected_pass, selected_fail

    by_stem_pass = {src.stem: src for src in pass_sources}
    by_stem_fail = {src.stem: src for src in fail_sources}

    for entry in entries:
        candidate = Path(entry)
        if candidate.exists():
            if "pass" in candidate.parts:
                selected_pass.append(candidate)
            elif "fail" in candidate.parts:
                selected_fail.append(candidate)
            else:
                selected_pass.append(candidate)
            continue

        in_pass = entry in by_stem_pass
        in_fail = entry in by_stem_fail

        if in_pass and not allow_pass:
            continue
        if in_fail and not allow_fail:
            continue

        if in_pass and in_fail and allow_pass and allow_fail:
            print(f"Ambiguous test name (exists in pass and fail): {entry}", file=sys.stderr)
            return [], []
        if in_pass:
            selected_pass.append(by_stem_pass[entry])
        elif in_fail:
            selected_fail.append(by_stem_fail[entry])
        else:
            print(f"Unknown test: {entry}", file=sys.stderr)
            return [], []

    return selected_pass, selected_fail


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", default=None)
    parser.add_argument("--list", action="store_true", help="List available compile tests")
    parser.add_argument("--pass", dest="only_pass", action="store_true", help="Run only passing tests")
    parser.add_argument("--fail", dest="only_fail", action="store_true", help="Run only failing tests")
    parser.add_argument("tests", nargs="*", help="Test names (stem) or paths")
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    build_dir = Path(args.build_dir) if args.build_dir else root / "build"
    cache_path = build_dir / "CMakeCache.txt"

    pass_sources = load_sources(root, "pass")
    fail_sources = load_sources(root, "fail")

    if args.only_pass and args.only_fail:
        print("Choose at most one of --pass or --fail", file=sys.stderr)
        return 1

    allow_pass = not args.only_fail
    allow_fail = not args.only_pass

    if args.list:
        if allow_pass:
            for src in pass_sources:
                print(f"pass:{src.stem}")
        if allow_fail:
            for src in fail_sources:
                print(f"fail:{src.stem}")
        return 0

    if not pass_sources and not fail_sources:
        print("No compile sources found.", file=sys.stderr)
        return 1

    if not cache_path.exists():
        print(f"Build dir not configured: {build_dir}", file=sys.stderr)
        print("Run CMake configure before invoking this script.", file=sys.stderr)
        return 1

    selected_pass, selected_fail = resolve_selection(
        args.tests, pass_sources, fail_sources, allow_pass, allow_fail
    )
    if not selected_pass and not selected_fail and args.tests:
        return 1

    cxx, std_value, extra_flags = compiler_settings(cache_path)
    compiler_id = detect_compiler_id(cxx)
    diag_flags: list[str] = []
    if compiler_id == "clang":
        diag_flags.append("-fcolor-diagnostics")
    elif compiler_id == "gcc":
        diag_flags.append("-fdiagnostics-color=always")

    expected_dir = root / "tests" / "cpp" / "compile" / "expected"
    actual_dir = build_dir / "test-outputs"
    actual_dir.mkdir(parents=True, exist_ok=True)

    failed = False

    for src in selected_pass:
        base = src.stem
        cmd = [
            cxx,
            f"-std=c++{std_value}",
            "-I",
            str(root / "include"),
            "-fsyntax-only",
            *diag_flags,
            *extra_flags,
            str(src),
        ]

        status, output = run_command(cmd, root, True)
        display = output.replace(str(root) + "/", "").replace(str(root), ".")
        if display:
            print(f"Compile output ({base}):", flush=True)
            print(display, end="" if display.endswith("\n") else "\n", flush=True)
            print()

        if status != 0:
            print(f"Expected compilation to succeed but failed: {src}", file=sys.stderr)
            failed = True

    for src in selected_fail:
        base = src.stem
        expected = expected_dir / f"{base}.txt"
        raw_path = actual_dir / f"{base}.raw.txt"
        actual_path = actual_dir / f"{base}.txt"

        cmd = [
            cxx,
            f"-std=c++{std_value}",
            "-I",
            str(root / "include"),
            "-fsyntax-only",
            *diag_flags,
            *extra_flags,
            str(src),
        ]

        status, output = run_command(cmd, root, True)
        raw_path.write_text(output, encoding="utf-8")

        normalized = normalize_output(output, root)
        actual_path.write_text(normalized, encoding="utf-8")

        display = output.replace(str(root) + "/", "").replace(str(root), ".")
        print(f"Compile output ({base}):", flush=True)
        print(display, end="" if display.endswith("\n") else "\n", flush=True)
        print()

        if status == 0:
            print(f"Expected compilation to fail but succeeded: {src}", file=sys.stderr)
            failed = True
            continue

        if not expected.exists():
            print(f"Missing expected output: {expected}", file=sys.stderr)
            print(f"Actual output written to: {actual_path}", file=sys.stderr)
            failed = True
            continue

        expected_text = expected.read_text(encoding="utf-8")
        if expected_text != normalized:
            print(f"Compile output mismatch for: {base}", file=sys.stderr)
            failed = True

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
