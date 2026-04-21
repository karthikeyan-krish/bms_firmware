#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

c_files() {
  git ls-files '*.c' '*.h'
}

cpp_files() {
  git ls-files '*.cc' '*.cpp' '*.cxx' '*.hpp' '*.hh'
}

format_c() {
  while IFS= read -r file; do
    [ -z "$file" ] && continue
    clang-format -i -style=file:tools/.clang-format-c --assume-filename="$file" "$file"
  done < <(c_files)
}

format_cpp() {
  while IFS= read -r file; do
    [ -z "$file" ] && continue
    clang-format -i -style=file:.clang-format --assume-filename="$file" "$file"
  done < <(cpp_files)
}

check_c() {
  while IFS= read -r file; do
    [ -z "$file" ] && continue
    clang-format --dry-run --Werror -style=file:tools/.clang-format-c --assume-filename="$file" "$file"
  done < <(c_files)
}

check_cpp() {
  while IFS= read -r file; do
    [ -z "$file" ] && continue
    clang-format --dry-run --Werror -style=file:.clang-format --assume-filename="$file" "$file"
  done < <(cpp_files)
}

case "${1:-}" in
  format)
    format_c
    format_cpp
    ;;
  check)
    check_c
    check_cpp
    ;;
  *)
    echo "Usage: $0 {format|check}"
    exit 1
    ;;
esac