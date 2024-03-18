import argparse
import os
import signal
import sys
import time
import subprocess

import lit.Test


class TestingProgressDisplay(object):
    def __init__(self, opts, numTests, progressBar=None):
        self.opts = opts
        self.numTests = numTests
        self.progressBar = progressBar
        self.completed = 0

    def finish(self):
        if self.progressBar:
            self.progressBar.clear()

    def update(self, test):
        self.completed += 1

        if self.progressBar:
            self.progressBar.update(float(self.completed)/self.numTests,
                                    test)

        if self.progressBar:
            self.progressBar.clear()

        # Show the test result line.
        print('%s (%d of %d)' % (test, self.completed, self.numTests))

        # Ensure the output is flushed.
        sys.stdout.flush()


def _execute_test_impl(test):
    """Execute one test"""

    result = None
    try:
        start_time = time.time()
        result = execute(test)
        if isinstance(result, tuple):
            code, output = result
        elapsed = time.time() - start_time
    except KeyboardInterrupt:
        raise


def executeCommand(command):
    p = subprocess.Popen(command,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    out,err = p.communicate()
    exitCode = p.wait()

    # Detect Ctrl-C in subprocess.
    if exitCode == -signal.SIGINT:
        raise KeyboardInterrupt

    # Ensure the resulting output is always of string type.
    try:
        out = str(out.decode('ascii'))
    except:
        out = str(out)
    try:
        err = str(err.decode('ascii'))
    except:
        err = str(err)
    return out, err, exitCode


def execute(test):
    """
    execute(test)
    :param test:
    :return:

    Form command for using CIRE_LLVM, execute it, and interpret the result.
    """
    cire_llvm = test.endswith('.ll')

    if cire_llvm:
        cmd = ['./CIRE_LLVM']
        if not os.path.isfile('backend-tv'):
            return lit.Test.UNSUPPORTED, ''

    out, err, exitCode = executeCommand(cmd)

    return lit.Test.PASS, ''


def execute_tests(display, tests):
    """
    execute_tests(display, tests)

    Execute each of the tests in the run, and inform the display of result.

    The display object will have its update method called with each test as
    it is completed.
    """
    # Don't do anything if we aren't going to run any tests.
    if not tests:
        return

    failure_count = 0

    for test_index, test in enumerate(tests):
        result = _execute_test_impl(test)

        (test_index, test_with_result) = result
        # Update. This includes the result, XFAILS, REQUIRES, and UNSUPPORTED statuses.
        tests[test_index] = test_with_result
        display.update(test_with_result)

        failure_count += (test_with_result.result.code == lit.Test.FAIL)


def main():
    parser = argparse.ArgumentParser(description='Run CIRE on directory')
    parser.add_argument('test_paths',
                        nargs='*',
                        help='Files or paths to run CIRE on')

    opts = parser.parse_args()
    args = opts.test_paths

    print("Running CIRE on: ", args)

    if not args:
        print("No test paths specified")

    progressBar = None
    startTime = time.time()
    display = TestingProgressDisplay(opts, len(args), progressBar)

    try:
        execute_tests(display, args)
    except KeyboardInterrupt:
        print("\nInterrupted")
    display.finish()
    testing_time = time.time() - startTime
