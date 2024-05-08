# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Test Timer class methods.
"""

import time

from py_utils.time.Timer import Timer


TEST_SLEEP_TIME = 0.5
TEST_RESIDUAL_TIME = 0.1


def test_ctor_trivial():
    """Create an object of the class and do nothing."""
    timer = Timer()  # noqa: F841


def test_elapsed():
    """Test elapsed method."""
    timer = Timer()
    time.sleep(TEST_SLEEP_TIME)
    elapsed = timer.elapsed()
    assert elapsed >= TEST_SLEEP_TIME and elapsed <= TEST_SLEEP_TIME + TEST_RESIDUAL_TIME


def test_reset():
    """Test reset method."""
    timer = Timer()
    time.sleep(TEST_SLEEP_TIME)
    timer.reset()
    elapsed = timer.elapsed()
    assert elapsed >= 0 and elapsed <= TEST_RESIDUAL_TIME
