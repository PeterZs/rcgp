#!/usr/bin/env python3

import subprocess
import sys
import re
from pathlib import Path

BOLD = "\033[1m"
RED = "\033[1;91m"
GREEN = "\033[1;92m"
RESET = "\033[0m"


def strip_paths(text):
	out = []
	for line in text.splitlines():
		out.append(re.sub(r'(\s*(?:In file included from\s+)?)\S*/', r'\1', line))
	return "\n".join(out) + "\n"


def strip_lowercase_ns(text):
	return re.sub(r'\b(?:[a-z][a-z0-9_]*::)+', '', text)


def drop_macro_expansion_notes(text):
	lines = text.splitlines(keepends=True)
	out = []
	i = 0
	while i < len(lines):
		if "note: expanded from macro" in lines[i]:
			i += 1
			while i < len(lines) and lines[i] and lines[i][0] in " \t":
				i += 1
		else:
			out.append(lines[i])
			i += 1
	return "".join(out)


def drop_error_summary(text):
	return re.sub(r'^\d+ errors? generated\.\n?', '', text, flags=re.MULTILINE)


def normalize_quotes(text):
	return text.replace("‘", "'").replace("’", "'")


def normalize(text):
	text = strip_paths(text)
	text = strip_lowercase_ns(text)
	text = drop_macro_expansion_notes(text)
	text = drop_error_summary(text)
	text = normalize_quotes(text)
	return text


def compiler_family(compiler):
	name = Path(compiler).name.lower()
	if "clang" in name:
		return "clang"
	if "g++" in name or "gcc" in name:
		return "gcc"
	return None


def main():
	if len(sys.argv) < 4:
		print(f"usage: {sys.argv[0]} <compiler> <cxxflags...> <source.cpp>", file=sys.stderr)
		sys.exit(2)

	compiler = sys.argv[1]
	cxxflags = sys.argv[2:-1]
	source = Path(sys.argv[-1])

	family = compiler_family(compiler)
	if family is None:
		print(f"{RED}failed:{RESET} {source}: cannot identify compiler family of {compiler!r}")
		sys.exit(2)

	expected_path = source.with_name(f"{source.stem}.{family}.stderr")
	if not expected_path.exists():
		print(f"{RED}failed:{RESET} {source}: missing expected output {expected_path}")
		sys.exit(1)

	result = subprocess.run(
		[compiler, *cxxflags, "-c", str(source), "-o", "/dev/null"],
		capture_output=True,
		text=True,
	)

	if result.returncode == 0:
		print(f"{RED}failed:{RESET} {source}: expected compilation to fail")
		sys.exit(1)

	actual_text = normalize(result.stderr)
	expected_text = normalize(expected_path.read_text())

	actual_lines = set(line for line in actual_text.splitlines() if line.strip())
	missing = [
		line for line in expected_text.splitlines()
		if line.strip() and line not in actual_lines
	]

	if not missing:
		print(f"{GREEN}passed:{RESET} {source}")
		sys.exit(0)

	print(f"{RED}failed:{RESET} {source}: expected diagnostics not present in actual output")
	sys.stderr.write(f"{BOLD}missing lines:{RESET}\n")
	for line in missing:
		sys.stderr.write(f"  {line}\n")
	sys.stderr.write(f"\n{BOLD}--- actual stderr ---{RESET}\n")
	sys.stderr.write(result.stderr)
	sys.exit(1)


if __name__ == "__main__":
	main()
