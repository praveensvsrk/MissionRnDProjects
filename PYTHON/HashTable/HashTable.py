import threading


class KeyValuePair(object):
    def __init__(self, key, value):
        self.key = key
        self.value = value

    def set_value(self, value):
        self.value = value

    def get_value(self):
        return self.value

    @property
    def key(self):
        return self.key

class LockedArray(object):
    lock_class = threading.Lock
    locks = None
    buckets = None
    size = 0
    def __init__(self, size):
        self.size = size
        self.locks = [self.lock_class()] * size
        self.buckets = [None] * size


class HashTable(LockedArray):
    def __init__(self, size):
        super(HashTable, self).__init__(size)

    def resize(self, new_size):
        if new_size == self.size or new_size < 0:
            return
        elif new_size < self.size:
            self.locks = self.locks[:new_size]
            self.buckets = self.buckets[:]

        else:
            self.locks += [self.lock_class()] * (new_size - self.size)


    def put(self, key, value):
        hash_loc = hash(key) % self.size
        if not self.buckets[hash_loc]:
            self.buckets[hash_loc] = [KeyValuePair(key, value)]
        else:
            for pair in self.buckets[hash_loc]:
                if pa