import os, time, threading

mutex = threading.Lock()
for file in os.listdir("testdata/"):
    mutex.acquire()
    start = time.time()
    os.system("./cmake-build-debug/PDSP < testdata/" + file + " > greedy_output/" + file)
    mutex.release()
    with open("greedy_output/" + file, "a") as f:
        f.write("Time: " + str(time.time() - start))
