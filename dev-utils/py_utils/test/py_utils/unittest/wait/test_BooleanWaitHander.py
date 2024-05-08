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
Test BooleanWaitHandler class methods.
"""

import time
import threading

from py_utils.wait.BooleanWaitHandler import BooleanWaitHandler
from py_utils.time.Timer import Timer


TEST_SLEEP_TIME = 0.5
TEST_RESIDUAL_TIME = TEST_SLEEP_TIME / 5


def test_ctor_trivial():
    """Test create an object of the class and to nothing."""
    waiter = BooleanWaitHandler(True, True)  # noqa: F841


def test_wait_condition_met():
    """Test to wait for condition met."""
    # Create waiter enabled
    waiter = BooleanWaitHandler(True, True)

    # Test to wait without waiting
    waiter.wait()

    # Close waiter
    waiter.close()

    # Create thread that sleep for a while and then disable the handler
    def condition_met_waiter_function(waiter):
        time.sleep(TEST_SLEEP_TIME)
        waiter.open()
    t = threading.Thread(target=condition_met_waiter_function, args=(waiter,))

    # Create a Timer to check is actually waiting
    timer = Timer()

    # Execute thread AFTER start timer (otherwise it could happen that elapsed time is lower)
    t.start()

    # Wait till condition met
    waiter.wait()

    # Get time elapsed waiting
    elapsed = timer.elapsed()

    # Check that such time is higher than wait time, and lower than it plus a residual time
    assert elapsed >= TEST_SLEEP_TIME and elapsed <= TEST_SLEEP_TIME + TEST_RESIDUAL_TIME
