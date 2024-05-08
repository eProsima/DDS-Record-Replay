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
BooleanWaitHandler class implementation.
"""

from py_utils.wait.WaitHandler import WaitHandler


class BooleanWaitHandler(WaitHandler):
    """
    BooleanWaitHandler is a WaitHandler that waits on a boolean value.
    Whenever this value is True, threads awake. This is also called open.
    Whenever it is False, threads wait. This is also called close.
    """

    def __init__(
            self,
            enabled: bool,
            init_value: bool):
        """
        BooleanWaitHandler ctor.

        Arguments:
            enabled (bool): whether the handler must start enabled or disabled.
            init_value (bool): whether the handler starts opened or closed.
        """
        super().__init__(enabled, init_value)

    def open(self):
        """Set internal value to True: awake all waiting threads."""
        self.set_value(True)

    def close(self):
        """Set internal value to False: new threads will wait."""
        self.set_value(False)

    def wait(
            self,
            timeout: float = None):
        """
        Wait till any thread open this object.

        Specialization of parent method with a specific predicate.

        Arguments:
            timeout: if thread awaken because timeout reached.
        """
        return super().wait(predicate=(lambda v: v), timeout=timeout)
