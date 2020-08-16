
import random


with open("stest3.txt", "w+") as file:

    file.write('1\n')

    n = 3000
    file.write('{}\n'.format(n))
    for num in range(n):
        a = num + 1
        b = 20000 - n + num
        file.write('{} {}'.format(a, b))
        if num != n - 1:
            file.write('\n')

