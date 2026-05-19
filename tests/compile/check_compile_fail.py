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


def normalize_auto_numbering(text):
	return re.sub(r'\bauto:\d+\b', 'auto:?', text)


def drop_gcc_elaboration_notes(text):
	patterns = ("evaluates to true", "evaluates to false", "the comparison reduces to")
	return "\n".join(
		line for line in text.splitlines()
		if not any(p in line for p in patterns)
	) + "\n"


def normalize(text):
	text = strip_paths(text)
	text = strip_lowercase_ns(text)
	text = drop_macro_expansion_notes(text)
	text = drop_error_summary(text)
	text = normalize_quotes(text)
	text = normalize_auto_numbering(text)
	text = drop_gcc_elaboration_notes(text)
	return text


def compiler_defines(compiler):
	name = Path(compiler).name.lower()
	if "clang" in name:
		return ["-D__clang__", "-U__GNUC__"]
	if "g++" in name or "gcc" in name:
		version = subprocess.run(
			[compiler, "-dumpversion"], capture_output=True, text=True,
		).stdout.strip().split(".")[0]
		defines = ["-U__clang__", "-D__GNUC__"]
		if version.isdigit():
			defines.append(f"-D__GNUC__={version}")
		return defines
	return []


def preprocess_golden(path, defines):
	args = ["unifdef", "-t"] + defines + [str(path)]
	result = subprocess.run(args, capture_output=True, text=True)
	# unifdef returns 0 if unchanged, 1 if changed; both are success.
	if result.returncode not in (0, 1):
		sys.stderr.write(result.stderr)
		sys.exit(2)
	return result.stdout


def main():
	if len(sys.argv) < 4:
		print(f"usage: {sys.argv[0]} <compiler> <cxxflags...> <source.cpp>", file=sys.stderr)
		sys.exit(2)

	compiler = sys.argv[1]
	cxxflags = sys.argv[2:-1]
	source = Path(sys.argv[-1])
	expected_path = source.with_suffix(".expected")

	if not expected_path.exists():
		print(f"{RED}failed:{RESET} {source}: missing companion file {expected_path}")
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
	expected_text = normalize(preprocess_golden(expected_path, compiler_defines(compiler)))

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
