import threading
import time

lock = threading.Lock()
arr = []
total = 0
results = []


def compute_sum(start, end, index):
    global total
    global arr
    global results
    local_sum = 0
    for i in range(start + 1, end + 1):
        local_sum += arr[i]

    results[index] = local_sum


def setup_array(size):
    global arr
    for i in range(size + 1):
        arr.append(i)


def threaded_sum():
    threads = []
    array_size = 1000000
    step = 500000
    global results
    results = [0] * (array_size/step)
    setup_array(array_size)
    j = 0
    for i in range(0, array_size, step):
        threads.append(threading.Thread(target=compute_sum, args=(i, i + step, j)))
        j += 1
    start = time.time()
    for t in threads:
        t.start()

    for t in threads:
        if t.isAlive():
            t.join()
    end = time.time()

    print "Threaded Sum of {} numbers is {}, time taken = {}".format(array_size, sum(results), end - start)


def normal_sum():
    start = time.time()
    total = 0
    for i in range(1000001):
        total += i
    end = time.time()
    print "Normal Sum of {} numbers is {}, time taken = {}".format(1000000, total, end - start)


if __name__ == '__main__':
    normal_sum()
    threaded_sum()