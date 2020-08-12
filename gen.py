
import random

num_nodes = 100000


def ihash(i):
    return (i*2654435761) % 0x7fffffff


class listh:

    def __init__(self, lis):
        self.lis = lis

    def __hash__(self):
        h = 859
        for el in self.lis:
            h = (h * 7919 + ihash(el)) % 0x7fffffff
        return h


with open("test4.txt", "w+") as file:
    tot = 1

    file.write(str(tot) + '\n')
    for i in range(tot):
        all = set()
        for num in range(1, num_nodes + 1):
            all.add(listh([num,]))

        file.write("{} {}\n".format(len(all), 1 ))

        while len(all) > 1:
            s1 = all.pop().lis
            s2 = all.pop().lis
            
            i1 = random.choice(s1)
            i2 = random.choice(s2)
            file.write("{} {} {}\n".format(i1, i2, 100000 ))

            all.add(listh(s1 + s2))



