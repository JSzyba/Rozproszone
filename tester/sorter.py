import re
import sys

# Messages to filter for 'short' mode
SHORT_MESSAGES = [
    "Zostałem postrzelony",
    "Żadanie o dok poszło do wszystkich",
    "Wylądowałem w doku",
    "Mechaników poszło do wszystkich",
    "start naprawy",
    "Koniec naprawy",
    "Uważam",
    "Naprawili"
]

SHORTEST_MESSAGES = [
    "sekcji",
]

def extract_lamport(line):
    match = re.search(r'lamport\s*=\s*(\d+)', line)
    return int(match.group(1)) if match else -1

def sort_by_lamport(input_file, output_file, short=False):
    with open(input_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # Filter only lines containing Lamport timestamps
    lines_with_lamport = [line for line in lines if 'lamport' in line]

    if short == 1:
        # Further filter to lines containing any of the SHORT_MESSAGES
        lines_with_lamport = [
            line for line in lines_with_lamport
            if any(msg in line for msg in SHORT_MESSAGES)
        ]

    elif short == 2:
                lines_with_lamport = [
            line for line in lines_with_lamport
            if any(msg in line for msg in SHORTEST_MESSAGES)
        ]
                
    # Sort by Lamport value (stable sort)
    sorted_lines = sorted(lines_with_lamport, key=extract_lamport)

    with open(output_file, 'w', encoding='utf-8') as f:
        f.writelines(sorted_lines)

    print(f"Sorted output written to {output_file}")

# Example usage
if __name__ == "__main__":
    if len(sys.argv) > 1:
        if sys.argv[1] == "short":
            short_mode = 1
        elif sys.argv[1] == "shortest":
            short_mode = 2
    else:
        short_mode = False

    sort_by_lamport('./tester/input.txt', './tester/sorted_output.txt', short=short_mode)
