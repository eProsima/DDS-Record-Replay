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
WaitHandler Parent class implementation.

WaitHandlers are classes that hold the needed variables and logic to make easy waits in conditions.
They encapsulate mutex, condition variables and logic to wait and to modify the value to wait for.
These classes allow to wait for a condition (any condition), and also stop with a timeout
or disabling the object.
"""

from enum import Enum
import threading


class AwakeReason(Enum):
    """Reason why a WaitHandler wait has finished."""
    disabled = 0,
    timeout = 1
    condition_met = 2


class WaitHandler:
    """
    WaitHandlers common abstract class.

    This class contains the mutex, condition variable and variable to perform wait.
    Child classes must implement wait method and add proper set methods.
    """

    def __init__(
            self,
            enabled: bool,
            init_value):
        """
        WaitHandler ctor.

        Arguments:
            enabled (bool): whether the handler must start enabled or disabled.
            init_value: any value that is stored inside.
        """
        self._value = init_value
        self._enabled = enabled
        self._lock = threading.Lock()
        self._condition = threading.Condition(self._lock)
        self._n_waiting_threads = 0

    # Get and set methods for the protected variable
    def get_value(self):
        """Get internal value"""
        with self._lock:
            return self._value

    def set_value(self, value):
        """Set internal value. It notifies possible waiting threads."""
        with self._lock:
            self._value = value
            self._condition.notify_all()

    # Enable/disable methods
    def enable(self):
        """Enable handler."""
        with self._lock:
            self._enable_nts()

    def disable(self):
        """Disable handler."""
        with self._lock:
            self._disable_nts()

    # Blocking disable method
    def blocking_disable(self):
        """Disable handler waiting for all waiting threads to have finished."""
        with self._lock:
            self._blocking_disable_nts()

    # Enabled method
    def enabled(self):
        """Whether the object is currently enabled."""
        with self._lock:
            return self._enabled_nts()

    def blank_predicate(_):
        """Default predicate to wait forever until timeout or disable."""
        return False

    # Wait method
    def wait(
            self,
            predicate=blank_predicate,
            timeout: float = None):
        """
        Wait for predicate to be fulfilled (True) or for timeout or object disabled.

        predicate must be a function that receives a value and returns True or False.
        The value to call predicate is the internal value of the WaitHandler, and it will be
        called every time this value changes.

        For instance, the default value for predicate is a function that never returns True.
        This means that thread will never exit until timeout reached or object disabled.
        This is useful to do a stoppable sleep.

        Arguments:
            predicate (function):
                predicate that returns True if thread must awake, False otherwise.
            timeout (float): max time in seconds to wait. None means no maximum time.

        Return:
            disabled: if thread awaken because object has been disabled.
            timeout: if thread awaken because timeout reached.
            condition_met: if thread awaken because condition have been fulfilled.

        Note:
            The priority for return values is the one set above.
            This means that, for instance, a thread that awakes because disabled and condition met
            at the same time, it will return disabled.
        """
        with self._lock:

            self._n_waiting_threads += 1

            awaken_reason = self._condition.wait_for(
                lambda: not self._enabled_nts() or predicate(self._value),
                timeout)

            self._n_waiting_threads -= 1

            if not self._enabled:
                return AwakeReason.disabled
            elif not awaken_reason:
                return AwakeReason.timeout
            else:
                return AwakeReason.condition_met

    # Enable/disable methods
    def _enable_nts(self):
        """Enable object. Not mutex safe, call with _lock taken."""
        self._enabled = True
        # It is not required to notify, as if it was disabled none thread would be waiting

    def _disable_nts(self):
        """Disable object. Not mutex safe, call with _lock taken."""
        self._enabled = False
        self._condition.notify_all()

    # Blocking disable method
    def _blocking_disable_nts(self):
        """Blocking disable object. Not mutex safe, call with _lock taken."""
        self._enabled = False
        while self._n_waiting_threads > 0:
            self._condition.notify_all()

    # Blocking disable method
    def _enabled_nts(self):
        """Whether object is enabled. Not mutex safe, call with _lock taken."""
        return self._enabled
