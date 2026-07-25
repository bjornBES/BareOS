#!/bin/sh
# gen_config.sh - Generate config.h and config.inc from config/config.env
#
# Usage:
#   ./gen_config.sh <input_env_file> <output_header_file> [output_asm_inc_file]
#
# Example:
#   ./gen_config.sh config/config.env src/kernel/include/kernel/config.h src/kernel/include/kernel/config.inc
#
# Rules:
#   - Lines starting with '#' or blank lines are skipped.
#   - Inline comments (anything after '#') are stripped from values.
#   - key = value  ->  #define CONFIG_<KEY_UPPER> value      (if value is 0/1 or numeric)
#                  ->  #define CONFIG_<KEY_UPPER> "value"    (otherwise, treated as string)
#   - Whitespace around key/value is trimmed.
#   - config.inc mirrors config.h using NASM %define, for use with %if/%ifdef.
#     Numeric values emit %define CONFIG_X value (usable in %if).
#     String values emit %define CONFIG_X (no value) so %ifdef still works,
#     since NASM %if cannot evaluate string literals the way C's #if can't either.

set -eu

IN="${1:-config/config.env}"
OUT="${2:-src/kernel/include/kernel/config.h}"
OUT_ASM="${3:-}"

if [ ! -f "$IN" ]; then
    echo "gen_config.sh: input file '$IN' not found" >&2
    exit 1
fi

OUT_DIR=$(dirname "$OUT")
mkdir -p "$OUT_DIR"

TMP=$(mktemp)

{
    echo "/* AUTO-GENERATED FILE - DO NOT EDIT BY HAND"
    echo " * Generated from $IN by gen_config.sh"
    echo " */"
    echo "#ifndef BAREOS_CONFIG_H"
    echo "#define BAREOS_CONFIG_H"
    echo
} > "$TMP"

TMP_ASM=""
if [ -n "$OUT_ASM" ]; then
    ASM_OUT_DIR=$(dirname "$OUT_ASM")
    mkdir -p "$ASM_OUT_DIR"
    TMP_ASM=$(mktemp)
    {
        echo "; AUTO-GENERATED FILE - DO NOT EDIT BY HAND"
        echo "; Generated from $IN by gen_config.sh"
        echo
    } > "$TMP_ASM"
fi

while IFS= read -r raw_line || [ -n "$raw_line" ]; do
    line="$raw_line"

    # Strip a full-line comment (line starts with # after trimming leading spaces)
    trimmed_check=$(printf '%s' "$line" | sed 's/^[[:space:]]*//')
    case "$trimmed_check" in
        '#'*|'') continue ;;
    esac

    # Must contain '='
    case "$line" in
        *=*) : ;;
        *) continue ;;
    esac

    key=$(printf '%s' "$line" | cut -d'=' -f1)
    val=$(printf '%s' "$line" | cut -d'=' -f2-)

    # Strip inline comment from value (anything after #)
    val=$(printf '%s' "$val" | sed 's/#.*//')

    # Trim leading/trailing whitespace from key and val
    key=$(printf '%s' "$key" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//')
    val=$(printf '%s' "$val" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//')

    [ -z "$key" ] && continue

    # Uppercase the key, replace any non-alnum with underscore
    key_upper=$(printf '%s' "$key" | tr '[:lower:]' '[:upper:]' | sed 's/[^A-Z0-9_]/_/g')

    # Decide numeric/bool vs string
    case "$val" in
        ''|*[!0-9]*)
            # Not purely digits -> treat as string
            echo "#define CONFIG_${key_upper} \"${val}\"" >> "$TMP"
            if [ -n "$TMP_ASM" ]; then
                # No value in NASM form -> %ifdef works, %if (value) does not
                echo "%define CONFIG_${key_upper}" >> "$TMP_ASM"
            fi
            ;;
        *)
            # Purely digits (0, 1, 250, etc.) -> emit raw
            echo "#define CONFIG_${key_upper} ${val}" >> "$TMP"
            if [ -n "$TMP_ASM" ]; then
                echo "%define CONFIG_${key_upper} ${val}" >> "$TMP_ASM"
            fi
            ;;
    esac
done < "$IN"

{
    echo
    echo "#endif /* BAREOS_CONFIG_H */"
} >> "$TMP"

mv "$TMP" "$OUT"
echo "gen_config.sh: wrote $OUT"

if [ -n "$OUT_ASM" ]; then
    mv "$TMP_ASM" "$OUT_ASM"
    echo "gen_config.sh: wrote $OUT_ASM"
fi