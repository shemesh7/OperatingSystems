import random

def random_string():
    ret = ''
    l = random.randint(1, 20)
    for i in range(l):
        ret += chr(i % (ord('z') - ord('a') + 1) + ord('a'))
    return ret

for __ in range(1000):
    x = random_string()
    l = len(x)
    top = ',\n    {\n'
    top += f'        "data": "{x}",\n        "requests": "'
    for _ in range(10):
        if random.randint(0, 1) == 0:
            a = random.randint(-2, l)
            top += fr'R {a} {random.randint(a, l + 2)}\n'
            print(fr'R {a} {random.randint(a, l + 2)}\n', end='')
        else:
            a = random.randint(-2, l)
            top += fr'W {a} {random_string()}\n' 
            print(fr'W {a} {random_string()}\n',end='')
    print('Q')
    top += 'Q"\n    }'
    print(top)
    with open('tests.json', 'a') as f:
        f.write(top)