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
Timer class implementation.
"""

import time


class Timer:
    """
    A simple timer class for measuring elapsed time.

    Attributes:
        _start_time (float): The start time of the timer in seconds since the epoch.

    NOTE: Chat GPT has written all this (including tests (⊙_⊙) )
    """

    def __init__(self):
        """
        Initializes a new Timer instance and sets the start time to the current time.
        """
        self._start_time = time.time()

    def reset(self):
        """
        Resets the timer by setting the start time to the current time.
        """
        self._start_time = time.time()

    def elapsed(self):
        """
        Calculates the elapsed time since the start time.

        Returns:
            float: The elapsed time in seconds.
        """
        return time.time() - self._start_time
