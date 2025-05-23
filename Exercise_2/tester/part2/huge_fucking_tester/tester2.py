import json
import subprocess

FILE_NAME = 'tests.json'
PROGRAM_NAME1 = './correct_file_processor'
PROGRAM_NAME2 = './file_processor'
all_passed = True
num_passed = 0

def show_diff(liel, noa, label):
    print(f"\n--- your output of {label}:")
    print(liel)
    print(f"\n+++  correct output of {label}:")
    print(noa)

with open(FILE_NAME) as f:
    tests = json.load(f)

for test_index, test in enumerate(tests, start=1):
    with open('data.txt', 'w') as f:
        f.write(test['data'])
    with open('requests.txt', 'w') as f:
        f.write(test['requests'])
    subprocess.run([PROGRAM_NAME1, 'data.txt', 'requests.txt'])
    with open('data.txt', 'r') as f:
        data1 = f.read()
    with open('read_results.txt', 'r') as f:
        read1 = f.read().rstrip('\n')

    with open('data.txt', 'w') as f:
        f.write(test['data'])
    with open('requests.txt', 'w') as f:
        f.write(test['requests'])
    subprocess.run([PROGRAM_NAME2, 'data.txt', 'requests.txt'])
    with open('data.txt', 'r') as f:
        data2 = f.read()
    with open('read_results.txt', 'r') as f:
        read2 = f.read().rstrip('\n')


    data_match = data1 == data2
    results_match = read1 == read2

    if data_match and results_match:
        print(f"✅ Test {test_index} passed")
        num_passed += 1
    else:
        all_passed = False
        print(f"\n❌ Test {test_index} failed")
        if not data_match:
            print("\n[data.txt mismatch]")
            show_diff(data1, data2, 'data.txt')

        if not results_match:
            print("\n[read_results.txt mismatch]")
            show_diff(read1, read2, 'read_results.txt')
    print('-' * 60)

if all_passed:
    print(r"""    .--.
   |o_o |
   |\_/ |
  //   \ \
 (|     | )
/'\_   _/`\
\___)=(___/
""")

else:
    print(f"you have passed {num_passed}/3096 tests.")
    