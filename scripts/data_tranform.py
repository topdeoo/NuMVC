import sys


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python data_tranform.py <input file> <output file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    with open(input_file, "r") as f:
        lines = f.readlines()

    n, m = map(int, lines[1].strip().split())
    nodes = []

    for line in lines[2: m + 2]:
        u, v = map(int, line.strip().split())
        nodes.append(u)
        nodes.append(v)

    nodes = list(set(nodes))

    with open(output_file, "w") as f:
        f.write(lines[0])
        f.write(str(n) + " " + str(m) + "\n")
        for node in nodes:
            f.write(str(node) + " ")
        f.write("\n")

        for line in lines[2: ]:
            f.write(line)
