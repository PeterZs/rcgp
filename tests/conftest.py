import os
from pathlib import Path

import pytest

from test_utils import run_command


def pytest_addoption(parser: pytest.Parser) -> None:
    parser.addoption(
        "--build-dir",
        action="store",
        default=None,
        help="CMake build directory (default: $RCGP_BUILD_DIR or ./build)",
    )


@pytest.fixture(scope="session")
def rcgp_root() -> Path:
    return Path(__file__).resolve().parent.parent


@pytest.fixture(scope="session")
def build_dir(pytestconfig: pytest.Config, rcgp_root: Path) -> Path:
    option = pytestconfig.getoption("--build-dir")
    env = os.environ.get("RCGP_BUILD_DIR")
    if option:
        return Path(option)
    if env:
        return Path(env)
    return rcgp_root / "build"


@pytest.fixture(scope="session")
def cmake_cache(build_dir: Path) -> Path:
    return build_dir / "CMakeCache.txt"


@pytest.fixture(scope="session")
def build_generated(rcgp_root: Path, build_dir: Path, cmake_cache: Path) -> None:
    if not cmake_cache.exists():
        pytest.fail(
            f"Build dir not configured: {build_dir}\n"
            "Run CMake configure before invoking pytest (or pass --build-dir)."
        )
    status, _ = run_command(
        ["cmake", "--build", str(build_dir), "--target", "rcgp_generated"],
        rcgp_root,
        False,
    )
    if status != 0:
        pytest.fail("Failed to build rcgp_generated target")


@pytest.fixture(scope="session")
def build_test_target(build_generated: None, rcgp_root: Path, build_dir: Path) -> None:
    status, _ = run_command(
        ["cmake", "--build", str(build_dir), "--target", "test"], rcgp_root, False
    )
    if status != 0:
        pytest.fail("Failed to build test target")
