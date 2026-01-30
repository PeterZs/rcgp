from pathlib import Path

import pytest

from test_utils import run_command


@pytest.mark.runtime
def test_runtime_suite(build_test_target: None, build_dir: Path, rcgp_root: Path) -> None:
    test_bin = build_dir / "test"
    if not test_bin.exists():
        pytest.fail(f"Test binary not found: {test_bin}")
    status, _ = run_command([str(test_bin)], rcgp_root, False)
    assert status == 0
