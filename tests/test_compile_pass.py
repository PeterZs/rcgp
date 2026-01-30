from pathlib import Path

import pytest

from test_utils import compiler_settings, detect_compiler_id, run_command


def pytest_generate_tests(metafunc: pytest.Metafunc) -> None:
    if "src" not in metafunc.fixturenames:
        return
    root = Path(__file__).resolve().parent.parent
    compile_dir = root / "tests" / "cpp" / "compile" / "pass"
    sources = sorted(compile_dir.glob("*.cpp"))
    if not sources:
        return
    metafunc.parametrize("src", sources, ids=[src.stem for src in sources])


@pytest.mark.compile_pass
@pytest.mark.usefixtures("build_generated")
def test_compile_pass_cases(src: Path, rcgp_root: Path, cmake_cache: Path) -> None:
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
    if status != 0:
        pytest.fail(f"Expected compilation to succeed but failed: {src}\n{output}")
