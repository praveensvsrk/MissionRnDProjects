from __future__ import print_function
import click
import os


def display_dash(level):
    if level > 0:
        ch = ' ' * level * 3
        print(ch, end='')
    print(chr(124), end='')


def display_dir(level, gen, remove, f):
    dir = next(gen)
    if f:
        for i in dir[2]:
            for s in remove:
                if s in i:
                    break
            else:
                display_dash(level)
                print('  ', i)
    if dir[1]:
        for i in dir[1]:
            for s in remove:
                if s in i:
                    break
            else:
                print()
                display_dash(level + 1)
                print(chr(196), end='')
                print(chr(196), end='')
                print(chr(196), end='')
                print(i)
            display_dir(level + 1, gen, remove, f)


@click.command()
@click.option('-r', '--remove', default='', help='Specifies substring to exclude', multiple = True)
@click.option('-f', is_flag = True, default=False, help='Specifies substring to exclude')
def tree(remove, f):
    gen = os.walk(os.getcwd())
    if remove:
        display_dir(0, gen, remove, f)
    else:
        display_dir(0, gen, '', f)


if __name__ == '__main__':
    tree()