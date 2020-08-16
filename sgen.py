
import random


with open("stest2.txt", "w+") as file:
    
    file.write('1\n')

    all = set()

    n = 30
    file.write('{}\n'.format(n))
    for num in range(n):
        while 1:
            a = random.randrange(1, 200001)
            b = random.randrange(1, 200001)
            if a > b:
                c = a
                a = b
                b = c

            if (a, b) in all:
                print((a, b), "twice")
                continue
            else:
                all.add((a, b))
                break
        file.write('{} {}'.format(a, b))
        if num != n - 1:
            file.write('\n')

