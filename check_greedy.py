import os, time, threading

'''
BUG(virgil) 
    1. Lack of input for standard answer will lead to an infinite loop of the program
    (测试文件中缺少需要输入的 `standard answer` 会导致程序无限循环)
    2. Wrong folder name
    (文件夹名字错误)
'''
mutex = threading.Lock()
for file in os.listdir("testdata/"):
    mutex.acquire()
    start = time.time()
    os.system("./cmake-build-debug/PDSP < testdata/" + file + " > greedy_output/" + file)
    mutex.release()
    with open("greedy_output/" + file, "a") as f:
        f.write("Time: " + str(time.time() - start))
