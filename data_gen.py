import os

for file in os.listdir("testdata/"):
    if file.startswith("case"):
        edge = set()
        with open("testdata/" + file, "r") as f:
            n, m = map(int, f.readline().split(' '))
            for _ in range(m):
                u, v = map(int, f.readline().split(' '))
                edge.add((u, v))
                edge.discard((v, u))
        with open("testdata/" + file, "w") as f:
            f.write(str(n) + " " + str(len(edge)) + "\n")
            for item in edge:
                f.write(str(item[0]) + " " + str(item[1]) + "\n")
