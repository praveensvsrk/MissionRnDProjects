from __future__ import print_function

class Node:
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None
        self.parent = None

    def set_data(self, data):
        self.data = data

    def get_data(self):
        return self.data

    def get_left_child(self):
        return self.left

    def get_right_child(self):
        return self.left

    def set_left_child(self, left):
        self. left = left

    def set_right_child(self, right):
        self.right = right

root = Node(0)


def build_tree_from_tuple(root, tup):
    if tup[1]:
        if type(tup[1]).__name__ == 'int':
            root.set_data(tup[1])
        else:
            root.set_data(build_tree_from_tuple(tup[1]))
    if tup[0]:
        if type(tup[0]).__name__ == 'int':
            root.left = Node(tup[0])
        else:
            root.left = Node(0)
            build_tree_from_tuple(root.left, tup[0])
    if tup[2]:
        if type(tup[2]).__name__ == 'int':
            root.right = Node(tup[2])
        else:
            root.right = Node(0)
            build_tree_from_tuple(root.right, tup[2])


def print_level_order(root):
    if root is None:
        return

    queue = []
    queue.append(root)

    while(len(queue) > 0):
        node = queue.pop(0)
        print(node.get_data())

        if not node.left is None:
            queue.append(node.left)

        if not node.right is None:
            queue.append(node.right)


def get_left_subtree_height(root):
    if root is None:
        return 0
    else:
        return 1 + get_left_subtree_height(root.left)


def get_right_subtree_height(root):
    if root is None:
        return 1
    else:
        return 1 + get_right_subtree_height(root.right)


def print_tree(root, level, is_right):
    if root is None:
        return
    lheight = get_left_subtree_height(root.left)
    if root.left:
        rheight = get_right_subtree_height(root.left.right)
    if is_right:
        print(' ' * 3 * level, end='')
    else:
        print(' ' * 3 * (level - 1), end='')
    print('--' * 3 * lheight, end='')
    print(root.get_data(), end='')
    if root.left:
        print('--' * 3 * rheight)
    print_tree(root.left, level + 1, False)
    print_tree(root.right, level + 1, True)



if __name__ == '__main__':
    tup = ((10, 30, 50), 20, (30, 40, (50, 60, None)))
    build_tree_from_tuple(root, tup)
    #print_level_order(root)
    print_tree(root, 0, True)