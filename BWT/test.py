import sys
import subprocess
import time
import re
import os

PROGRAM_NAME = './bwtsearch'
INPUT_FOLDER = 'testfiles'
TIME_LIMIT = 30
TIME_LIMIT_FIRST = 120
RESULT_PATTERN = re.compile(r'(\[\d+\][^\[\]]+)\n')
TESTS = (
    ('simple1', '['),
    ('simple1', ']'),
    ('simple1', '[2]'),
    ('simple1', 'z'),
    ('simple1', 'b', 'z'),
    ('simple1', 'b'),
    ('simple1', 'n'),
    ('simple1', 'ana'),
    ('simple1', 'banana'),
    ('simple2', 'ban'),
    ('simple2', 'banana'),
    ('simple2', 'ba', 'na'),
    ('simple3', 'n'),
    ('simple3', 'ban'),
    ('simple3', 'and'),
    ('simple3', 'n', 'a'),
    ('simple3', 'gan'),
    ('simple4', 'ban'),
    ('simple4', 'bind', 'ing'),
    ('simple4', 'bind', 'ing', 'ban'),
    ('tiny', "'RISC' "),
    ('tiny', '9'),
    ('tiny', '9', '9'),
    ('tiny', '9', '9', '9'),
    ('tiny', 'k M'),
    ('shopping', 'co'),
    ('shopping', '$'),
    ('shopping', '$', '.00'),
    ('dblp', 'sys'),
    ('dblp', 'sys', 'comp'),
    ('dblp', 'sys', 'comp', ' d'),
    ('large', 'user requirement'),
    ('large', 'Multiobject'),
    ('large', 'noi', 'g')
)
NAME_SET = set()
TEST_ID = 1 # just for displaying use, doesn't mean which case is chosen


class Colors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[36m'
    GREEN = '\033[92m'
    WARN = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


class TestException(Exception):
    pass


def print_color(color_code, *args, **kwargs):
    print(color_code, end='')
    print(*args, **kwargs)
    print(Colors.ENDC, end='')


def print_start(cmd):
    print_color(Colors.HEADER, '=== Test %d ===' % TEST_ID)
    print_color(Colors.BLUE, cmd)


def print_passed(time_interval):
    global TEST_ID
    print_color(Colors.GREEN, 'Passed:', '%.3fs' % time_interval)
    TEST_ID += 1


def print_failed(reason):
    global TEST_ID
    print_color(Colors.FAIL, 'Failed:', reason)
    TEST_ID += 1


def mismatch(message, expected, output):
    print_failed(message)
    if expected is not None:
        print_color(Colors.FAIL,'Expected:', expected)
    print_color(Colors.FAIL,'Output:', output)
    raise TestException()


def non_zero_exit(return_code, output):
    print_failed('Non zero exit code %d' % return_code)
    print_color(Colors.FAIL, output)


def timeout(time_interval, first):
    if (not first and time_interval >= TIME_LIMIT or
            first and time_interval >= TIME_LIMIT_FIRST):
        print_failed('Timeout')
        limit = TIME_LIMIT_FIRST if first else TIME_LIMIT
        print_color(Colors.FAIL, 'Limit: %ds' % limit)
        print_color(Colors.FAIL, 'Time: %.3fs' % time_interval)
        return True
    return False


def record_id(record):
    right = record.find(']')
    try:
        return int(record[1:right])
    except:
        print(record)
        raise


def check_line(results, index, truth_line, search_terms, last=False):
    truth_line = '[' + truth_line.decode()
    if not last:
        truth_line = truth_line[:-1]
    left = truth_line.find(']') + 1
    if all( term in truth_line[left:] for term in search_terms ):
        if index >= len(results):
            mismatch('Too few results', None, len(results))
        if results[index] != truth_line:
            mismatch('Mismatch line', truth_line, results[index])
        return True
    return False


def shell_escape(s):
    s = s.replace('\\', '\\\\')
    s = s.replace('"', '\\"')
    s = s.replace('\n', '\\n')
    s = s.replace('\r', '\\r')
    s = s.replace('\t', '\\t')
    s = s.replace('\v', '\\v')
    return '"' + s + '"'


def test(name, *args):
    first = not name in NAME_SET
    NAME_SET.add(name)

    bwt_file_name = os.path.join(INPUT_FOLDER, name + '.bwt')
    idx_file_name = os.path.join(INPUT_FOLDER, name + '.idx')
    txt_file_name = os.path.join(INPUT_FOLDER, name + '.txt')

    cmd = [PROGRAM_NAME, bwt_file_name, idx_file_name]
    print_start(' '.join(cmd + list(map(shell_escape, args))))
    if args:
        cmd.extend(args)

    time_start = time.time()
    try:
        output = subprocess.check_output(cmd, stderr=subprocess.PIPE)
    except subprocess.CalledProcessError as e:
        non_zero_exit(e.returncode, e.stderr.decode())
        return False
    time_interval = time.time() - time_start
    if timeout(time_interval, first):
        return False

    results = RESULT_PATTERN.findall(output.decode())
    results.sort(key=record_id)
    size_results = len(results)
    results[:] = [results[i]
                  for i in range(size_results)
                  if i == 0 or
                     record_id(results[i]) != record_id(results[i-1])]

    try:
        fp = open(txt_file_name)
        cmd = ['tr', '[', '\n']
        tr = subprocess.Popen(cmd, stdin=fp, stdout=subprocess.PIPE)

        i = 0
        truth_line = ''
        for j, prev_line in enumerate(tr.stdout):
            if j >= 2 and check_line(results, i, truth_line, args):
                    i += 1
            truth_line = prev_line
        if check_line(results, i, truth_line, args, last=True):
            i += 1

        if i < len(results):
            mismatch('Too many results', i, size_results)

    except TestException:
        return False
    except:
        raise
    finally:
        fp.close()

    print_passed(time_interval)
    return True


def all_tests():
    cnt_passed = 0
    cnt_failed = 0
    for test_params in TESTS:
        if test(*test_params):
            cnt_passed += 1
        else:
            cnt_failed += 1

    print()
    if cnt_failed == 0:
        print_color(Colors.GREEN, '!!! ALL PASSED !!!')
    else:
        print_color(Colors.FAIL,
                    '!!! %d PASSED, %d FAILED !!!' % (cnt_passed, cnt_failed))
    print()


if __name__ == '__main__':
    all_tests()
