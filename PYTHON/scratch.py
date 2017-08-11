def outer():
    def inner():
        print "inner"
    return inner

res = []


class mycl:
    def __init__(self, a):
        self.a = a


def myfunc():
    return 5
res = []

for i in range(8):
    o = outer()
    print id(o)
    o = outer()
    print id(o)
    o = outer()
    print id(o)
    o = outer()
    print id(o)

# for i in res:
#     print id(i)