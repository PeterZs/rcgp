from pathlib import Path

import pytest

from test_utils import (
    compiler_settings,
    detect_compiler_id,
    diff_text,
    normalize_output,
    run_command,
)


@pytest.fixture(scope="session")
def compile_expected_dir(rcgp_root: Path) -> Path:
    return rcgp_root / "tests" / "cpp" / "compile" / "expected"


@pytest.fixture(scope="session")
def compile_actual_dir(build_dir: Path) -> Path:
    actual_dir = build_dir / "test-outputs"
    actual_dir.mkdir(parents=True, exist_ok=True)
    return actual_dir


def pytest_generate_tests(metafunc: pytest.Metafunc) -> None:
    if "src" not in metafunc.fixturenames:
        return
    root = Path(__file__).resolve().parent.parent
    compile_dir = root / "tests" / "cpp" / "compile" / "fail"
    sources = sorted(compile_dir.glob("*.cpp"))
    if not sources:
        return
    metafunc.parametrize("src", sources, ids=[src.stem for src in sources])


@pytest.mark.compile_fail
@pytest.mark.usefixtures("build_generated")
def test_compile_fail_cases(
    src: Path,
    rcgp_root: Path,
    cmake_cache: Path,
    compile_expected_dir: Path,
    compile_actual_dir: Path,
) -> None:
    if not cmake_cache.exists():
        pytest.fail(
            f"Build dir not configured: {cmake_cache.parent}\n"
            "Run CMake configure before invoking pytest (or pass --build-dir)."
        )

    cxx, std_value, extra_flags = compiler_settings(cmake_cache)
    compiler_id = detect_compiler_id(cxx)
    diag_flags: list[str] = []
    if compiler_id == "clang":
        diag_flags.append("-fcolor-diagnostics")
    elif compiler_id == "gcc":
        diag_flags.append("-fdiagnostics-color=always")

    expected = compile_expected_dir / f"{src.stem}.txt"
    raw_path = compile_actual_dir / f"{src.stem}.raw.txt"
    actual_path = compile_actual_dir / f"{src.stem}.txt"

    cmd = [
        cxx,
        f"-std=c++{std_value}",
        "-I",
        str(rcgp_root / "include"),
        "-fsyntax-only",
        *diag_flags,
        *extra_flags,
        str(src),
    ]

    status, output = run_command(cmd, rcgp_root, True)
    raw_path.write_text(output, encoding="utf-8")

    normalized = normalize_output(output, rcgp_root)
    actual_path.write_text(normalized, encoding="utf-8")

    if status == 0:
        pytest.fail(f"Expected compilation to fail but succeeded: {src}")

    if not expected.exists():
        pytest.fail(f"Missing expected output: {expected}\nActual output written to: {actual_path}")

    expected_text = expected.read_text(encoding="utf-8")
    if expected_text != normalized:
        diff = diff_text(expected_text, normalized, expected, actual_path)
        pytest.fail(f"Compile output mismatch for: {src.stem}\n{diff}")
