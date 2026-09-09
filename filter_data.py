import argparse
import re
from pathlib import Path


ANSI_ESCAPE_RE = re.compile(r"\x1B\[[0-?]*[ -/]*[@-~]")

# Keep the original line order and only retain the requested data items.
TARGET_PATTERNS = (
    re.compile(r"^ADC\b"),
    re.compile(r"\bgas:"),
    re.compile(r"\bFlow:"),
    re.compile(r"\bCurrent(?:\[\d+\])?:"),
    re.compile(r"\bVoltage(?:\[\d+\])?:"),
)


def should_keep(line: str) -> bool:
    return any(pattern.search(line) for pattern in TARGET_PATTERNS)


def filter_lines(lines: list[str]) -> list[str]:
    kept: list[str] = []
    for raw_line in lines:
        line = ANSI_ESCAPE_RE.sub("", raw_line).strip()
        if line and should_keep(line):
            kept.append(line)
    return kept


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Filter data.txt and keep only ADC, gas, Flow, Current, and Voltage lines."
    )
    parser.add_argument(
        "-i",
        "--input",
        default="data.txt",
        help="Input log file path. Defaults to data.txt.",
    )
    parser.add_argument(
        "-o",
        "--output",
        default="filtered_data.txt",
        help="Output file path. Defaults to filtered_data.txt.",
    )
    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)

    lines = input_path.read_text(encoding="utf-8", errors="ignore").splitlines()
    filtered_lines = filter_lines(lines)
    output_path.write_text("\n".join(filtered_lines) + ("\n" if filtered_lines else ""), encoding="utf-8")

    print(f"Input: {input_path}")
    print(f"Output: {output_path}")
    print(f"Matched lines: {len(filtered_lines)}")


if __name__ == "__main__":
    main()
