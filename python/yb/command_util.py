#
# Copyright (c) YugaByte, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
# in compliance with the License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under the License
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
# or implied.  See the License for the specific language governing permissions and limitations
# under the License.
#
"""
Copyright (c) YugaByte, Inc.

This module provides utilities for running commands.
"""

import os
import subprocess
import logging

from collections import namedtuple


ProgramResult = namedtuple('ProgramResult',
                           ['cmd_line',
                            'returncode',
                            'stdout',
                            'stderr',
                            'error_msg',
                            'program_path'])


def run_program(args, error_ok=False):
    """
    Run the given program identified by its argument list, and return a ProgramResult object.

    @param error_ok True to raise an exception on errors.
    """
    if not isinstance(args, list):
        args = [args]
    try:
        program_subprocess = subprocess.Popen(
            args,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
    except OSError:
        logging.error("Failed to run program {}".format(args))
        raise

    program_stdout, program_stderr = program_subprocess.communicate()
    error_msg = None
    if program_subprocess.returncode != 0:
        error_msg = "Non-zero exit code {} from: {}, stdout: '{}', stderr: '{}'".format(
                program_subprocess.returncode, args,
                program_stdout.strip(), program_stderr.strip())
        if not error_ok:
            raise RuntimeError(error_msg)
    return ProgramResult(cmd_line=args,
                         program_path=os.path.realpath(args[0]),
                         returncode=program_subprocess.returncode,
                         stdout=program_stdout.strip(),
                         stderr=program_stderr.strip(),
                         error_msg=error_msg)
