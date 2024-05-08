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
Test IntWaitHandler class methods.
"""

import time
import threading

from py_utils.wait.IntWaitHandler import IntWaitHandler
from py_utils.time.Timer import Timer


TEST_SLEEP_TIME = 0.5
TEST_RESIDUAL_TIME = TEST_SLEEP_TIME / 5


def test_ctor_trivial():
    """Test create an object of the class and to nothing."""
    waiter = IntWaitHandler(True)  # noqa: F841


def test_wait_condition_met():
    """Test to wait for condition met."""
    # Create waiter enabled with value expected
    waiter = IntWaitHandler(True)

    # Test to wait without waiting
    waiter.wait_equal(0)

    # Create thread that sleep for a while and then disable the handler
    def condition_met_waiter_function(waiter):

        # Wait and increase in 1
        time.sleep(TEST_SLEEP_TIME)
        waiter.increase()

        # Wait and increase in 2
        time.sleep(TEST_SLEEP_TIME)
        waiter.increase(2)

        # Wait and decrease 3
        time.sleep(TEST_SLEEP_TIME)
        waiter.decrease(3)

    t = threading.Thread(target=condition_met_waiter_function, args=(waiter,))

    # Create a Timer to check is actually waiting
    timer = Timer()

    # Execute thread AFTER start timer (otherwise it could happen that elapsed time is lower)
    t.start()

    # Wait till condition met
    waiter.wait_greater_equal(1)
    waiter.wait_greater(2)
    waiter.wait_equal(0)

    # Get time elapsed waiting
    elapsed = timer.elapsed()

    # Check that such time is in range expected
    assert elapsed >= TEST_SLEEP_TIME * 3 and elapsed <= TEST_SLEEP_TIME * 3 + TEST_RESIDUAL_TIME
