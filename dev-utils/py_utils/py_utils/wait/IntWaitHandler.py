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
IntWaitHandler class implementation.
"""

from py_utils.wait.WaitHandler import WaitHandler


class IntWaitHandler(WaitHandler):
    """
    IntWaitHandler is a WaitHandler that waits on a int value.
    It implements functions to wait for a specific value, or wait for the value to
    be greater or greater equal.
    It also implements functions to increase or decrease the internal value.

    This is useful, for instance, to wait for an event to happen N or more times.

    For more specific set or wait methods, implement a child class.
    """

    def __init__(
            self,
            enabled: bool,
            init_value: int = 0):
        """
        IntWaitHandler ctor.

        Arguments:
            enabled (bool): whether the handler must start enabled or disabled.
            init_value (int): initial value of the internal integer.
        """
        super().__init__(enabled, init_value)

    def increase(self, value: int = 1):
        """Increment the internal integer in value."""
        with self._lock:
            self._value += value
            self._condition.notify_all()

    def decrease(self, value: int = 1):
        """Decrease the internal integer in value."""
        with self._lock:
            self._value -= value
            self._condition.notify_all()

    def wait_equal(self, value, timeout=None):
        """
        Wait till internal object is equal the given value.

        Specialization of parent method with a specific predicate.

        Arguments:
            value: value to wait to be equal the internal integer.
            timeout: if thread awaken because timeout reached.
        """
        return super().wait(predicate=(lambda v: value == v), timeout=timeout)

    def wait_greater_equal(self, value, timeout=None):
        """Wait for the internal integer to be greater or equal the value given."""
        return super().wait(predicate=(lambda v: v >= value), timeout=timeout)

    def wait_greater(self, value, timeout=None):
        """Wait for the internal integer to be greater than value given."""
        return super().wait(predicate=(lambda v: v > value), timeout=timeout)
