import time
from functools import wraps, update_wrapper


def profile_count(skip_recursion=True):
    def outer(func):
        if skip_recursion:
            def inner(*args):
                return func(*args)
            inner.count = 1
        else:
            def inner(*args):
                inner.count += 1
                return func(*args)
            inner.count = 0
        return inner
    return outer


def profile_time(func):
    @wraps(func)
    def inner(*args):
        start = time.time()
        result = func(*args)
        end = time.time()
        inner.time_taken = end - start
        update_wrapper(inner, func)
        return result
    return inner


@profile_count(skip_recursion=True)
def fib(n):
    if n <= 0:
        return 0
    if n == 1 or n == 2:
        return 1
    return fib(n-1) + fib(n-2)


@profile_time
@profile_count(skip_recursion=False)
def fib2(n):
    if n <= 0:
        return 0
    if n == 1 or n == 2:
        return 1
    return fib2(n-1) + fib2(n-2)


@profile_time
def fib3(n):
    if n <= 0:
        return 0
    if n == 1 or n == 2:
        return 1
    return fib3(n-1) + fib3(n-2)


if __name__ == '__main__':
    fib(5)
    print("Skipping recursion ", fib.count)
    fib2(5)
    print("Without skipping recursion ", fib2.count)
    fib3(20)
    print(fib2.time_taken)