#!/usr/bin/env bash
# A test suite for the shell implementation in shell.c.

set -u
set -o pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TMP_DIR="$(mktemp -d)"
WORK_DIR="$TMP_DIR/work"
FAKE_BIN="$TMP_DIR/fakebin"
SHELL_BIN="$TMP_DIR/shell"
LOG_FILE="$TMP_DIR/cmd.log"

tests_run=0
tests_failed=0

cleanup() {
  rm -rf "$TMP_DIR"
}
trap cleanup EXIT

mkdir -p "$WORK_DIR" "$FAKE_BIN"
: > "$LOG_FILE"

if ! gcc "$ROOT_DIR/shell.c" -o "$SHELL_BIN"; then
  echo "Failed to compile shell.c"
  exit 1
fi

cat > "$FAKE_BIN/cp" <<'EOF'
#!/usr/bin/env bash
echo "cp:$*" >> "$TEST_LOG"
/bin/cp "$@"
EOF

cat > "$FAKE_BIN/rm" <<'EOF'
#!/usr/bin/env bash
echo "rm:$*" >> "$TEST_LOG"
/bin/rm "$@"
EOF

cat > "$FAKE_BIN/more" <<'EOF'
#!/usr/bin/env bash
echo "more:$*" >> "$TEST_LOG"
/bin/cat "$@"
EOF

cat > "$FAKE_BIN/nano" <<'EOF'
#!/usr/bin/env bash
echo "nano:$*" >> "$TEST_LOG"
if [ "$#" -ge 1 ]; then
  touch "$1"
fi
EOF

cat > "$FAKE_BIN/clear" <<'EOF'
#!/usr/bin/env bash
echo "clear:$*" >> "$TEST_LOG"
printf "FAKE_CLEAR\n"
EOF

cat > "$FAKE_BIN/pwd" <<'EOF'
#!/usr/bin/env bash
echo "pwd:$*" >> "$TEST_LOG"
printf "FAKE_PWD\n"
EOF

cat > "$FAKE_BIN/ls" <<'EOF'
#!/usr/bin/env bash
echo "ls:$*" >> "$TEST_LOG"
printf "FAKE_LS:%s\n" "$*"
EOF

cat > "$FAKE_BIN/xcmd" <<'EOF'
#!/usr/bin/env bash
echo "xcmd:$*" >> "$TEST_LOG"
printf "XCMD:%s\n" "$*"
EOF

chmod +x "$FAKE_BIN"/*

run_shell() {
  local input="$1"
  (
    cd "$WORK_DIR" || exit 1
    printf "%b" "$input" | env PATH="$FAKE_BIN:/usr/bin:/bin" TEST_LOG="$LOG_FILE" "$SHELL_BIN"
  )
}

reset_log() {
  : > "$LOG_FILE"
}

pass() {
  printf "PASS: %s\n" "$1"
}

fail() {
  printf "FAIL: %s\n" "$1"
  tests_failed=$((tests_failed + 1))
}

assert_contains() {
  local haystack="$1"
  local needle="$2"
  local desc="$3"
  tests_run=$((tests_run + 1))
  if [[ "$haystack" == *"$needle"* ]]; then
    pass "$desc"
  else
    fail "$desc (missing: $needle)"
  fi
}

assert_not_contains() {
  local haystack="$1"
  local needle="$2"
  local desc="$3"
  tests_run=$((tests_run + 1))
  if [[ "$haystack" == *"$needle"* ]]; then
    fail "$desc (unexpected: $needle)"
  else
    pass "$desc"
  fi
}

assert_file_exists() {
  local file="$1"
  local desc="$2"
  tests_run=$((tests_run + 1))
  if [ -f "$file" ]; then
    pass "$desc"
  else
    fail "$desc (file not found: $file)"
  fi
}

assert_file_not_exists() {
  local file="$1"
  local desc="$2"
  tests_run=$((tests_run + 1))
  if [ ! -e "$file" ]; then
    pass "$desc"
  else
    fail "$desc (file still exists: $file)"
  fi
}

assert_log_contains() {
  local needle="$1"
  local desc="$2"
  tests_run=$((tests_run + 1))
  if grep -Fq "$needle" "$LOG_FILE"; then
    pass "$desc"
  else
    fail "$desc (missing log: $needle)"
  fi
}

# Q exits
reset_log
output="$(run_shell $'Q\n' 2>&1 || true)"
assert_contains "$output" "shell: Terminating successfully" "Q exits the shell"

# H shows help manual
reset_log
output="$(run_shell $'H\nQ\n' 2>&1 || true)"
assert_contains "$output" "Shell User Manual" "H prints help text"

# E echoes arguments
reset_log
output="$(run_shell $'E Hello Testing\nQ\n' 2>&1 || true)"
assert_contains "$output" "Hello Testing" "E prints arguments"

# C maps to cp
reset_log
printf "copy-source\n" > "$WORK_DIR/source.txt"
output="$(run_shell $'C source.txt copied.txt\nQ\n' 2>&1 || true)"
assert_file_exists "$WORK_DIR/copied.txt" "C creates destination file"
assert_log_contains "cp:source.txt copied.txt" "C invokes cp"
if [ -f "$WORK_DIR/copied.txt" ]; then
  tests_run=$((tests_run + 1))
  if [ "$(cat "$WORK_DIR/copied.txt")" = "copy-source" ]; then
    pass "C copies file contents"
  else
    fail "C copies file contents"
  fi
fi

# D maps to rm
reset_log
printf "delete-me\n" > "$WORK_DIR/todelete.txt"
output="$(run_shell $'D todelete.txt\nQ\n' 2>&1 || true)"
assert_file_not_exists "$WORK_DIR/todelete.txt" "D removes file"
assert_log_contains "rm:todelete.txt" "D invokes rm"

# M maps to nano
reset_log
output="$(run_shell $'M notes.txt\nQ\n' 2>&1 || true)"
assert_log_contains "nano:notes.txt" "M invokes nano"
assert_file_exists "$WORK_DIR/notes.txt" "M passes filename to nano"

# M missing argument
reset_log
output="$(run_shell $'M\nQ\n' 2>&1 || true)"
assert_contains "$output" "M: missing file name" "M reports missing filename"

# P maps to more
reset_log
printf "page-content\n" > "$WORK_DIR/page.txt"
output="$(run_shell $'P page.txt\nQ\n' 2>&1 || true)"
assert_log_contains "more:page.txt" "P invokes more"
assert_contains "$output" "page-content" "P displays file content"

# W maps to clear
reset_log
output="$(run_shell $'W\nQ\n' 2>&1 || true)"
assert_log_contains "clear:" "W invokes clear"
assert_contains "$output" "FAKE_CLEAR" "W shows clear output"

# L custom command (pwd + ls -l)
reset_log
output="$(run_shell $'L\nQ\n' 2>&1 || true)"
assert_contains "$output" "FAKE_PWD" "L runs pwd"
assert_contains "$output" "FAKE_LS:-l" "L runs ls -l"
assert_log_contains "pwd:" "L logs pwd invocation"
assert_log_contains "ls:-l" "L logs ls -l invocation"

# X executes external command
reset_log
output="$(run_shell $'X xcmd alpha beta\nQ\n' 2>&1 || true)"
assert_contains "$output" "XCMD:alpha beta" "X runs external command"
assert_log_contains "xcmd:alpha beta" "X passes arguments to external command"

# X executes custom command (regression test for X L)
reset_log
output="$(run_shell $'X L\nQ\n' 2>&1 || true)"
assert_contains "$output" "FAKE_PWD" "X L reaches custom L behavior"
assert_contains "$output" "FAKE_LS:-l" "X L runs ls -l via L"
assert_not_contains "$output" "No such file or directory" "X L does not try execvp on literal L"

# X missing argument
reset_log
output="$(run_shell $'X\nQ\n' 2>&1 || true)"
assert_contains "$output" "X: missing program name" "X reports missing program name"

passed=$((tests_run - tests_failed))
printf "\nTest summary: %d passed, %d failed, %d total\n" "$passed" "$tests_failed" "$tests_run"

if [ "$tests_failed" -ne 0 ]; then
  exit 1
fi
