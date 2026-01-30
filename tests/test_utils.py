import difflib
import os
import re
import shlex
import subprocess
from pathlib import Path

ANSI_RE = re.compile(r"\x1b\[[0-9;]*[mK]")


def strip_ansi(text: str) -> str:
    return ANSI_RE.sub("", text)


def normalize_output(text: str, root: Path) -> str:
    stripped = strip_ansi(text)
    root_str = str(root)
    return stripped.replace(root_str + "/", "").replace(root_str, ".")


def read_cmake_cache_value(cache_path: Path, key: str) -> str | None:
    if not cache_path.exists():
        return None
    prefix = f"{key}:"
    with cache_path.open("r", encoding="utf-8") as handle:
        for line in handle:
            if line.startswith(prefix):
                return line.split("=", 1)[1].strip()
    return None


def detect_compiler_id(compiler: str) -> str:
    try:
        proc = subprocess.run(
            [compiler, "--version"],
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
    except FileNotFoundError:
        return "unknown"
    output = (proc.stdout or "").lower()
    if "clang" in output:
        return "clang"
    if "gcc" in output or "g++" in output:
        return "gcc"
    return "unknown"


def run_command(cmd: list[str], cwd: Path, capture: bool) -> tuple[int, str]:
    if capture:
        proc = subprocess.run(
            cmd,
            cwd=cwd,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        return proc.returncode, proc.stdout or ""
    returncode = subprocess.call(cmd, cwd=cwd)
    return returncode, ""


def compiler_settings(cache_path: Path) -> tuple[str, str, list[str]]:
    cxx = os.environ.get("CXX") or read_cmake_cache_value(cache_path, "CMAKE_CXX_COMPILER") or "c++"
    std_value = read_cmake_cache_value(cache_path, "CMAKE_CXX_STANDARD") or "26"
    cxx_flags = read_cmake_cache_value(cache_path, "CMAKE_CXX_FLAGS") or ""
    extra_flags = shlex.split(cxx_flags)
    return cxx, std_value, extra_flags


def diff_text(expected: str, actual: str, expected_path: Path, actual_path: Path) -> str:
    diff = difflib.unified_diff(
        expected.splitlines(),
        actual.splitlines(),
        fromfile=str(expected_path),
        tofile=str(actual_path),
        lineterm="",
    )
    return "\n".join(diff)
