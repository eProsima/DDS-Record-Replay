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
This file contains utils to use native python Log.

Implement a Custom Logger class and creates a global object of such class.
In order to use this log handler, import logger object from this module and use it
as any other Logger:
- logger.info(...)
- logger.debug(...)
- logger.log(logging.DEBUG, ...)

In order to set a different log level, use set_log_level or activate_debug logger methods.
"""

import logging


class CustomLogger(logging.Logger):
    """
    Class to create a global Logger shared by the whole process.
    It inherits from native logging.Logger and implement auxiliary methods.
    """

    def __init__(
            self,
            log_level=logging.INFO,
            logger_name: str = 'GLOBAL_LOG'):
        """Creates an object of Logger class and set log level and default format."""
        super().__init__(logger_name)
        self.set_format()
        self.set_log_level(log_level)

    def set_format(
            self,
            format: str = '[%(asctime)s][%(name)s][%(levelname)s] %(message)s'):
        """Modifies format for logger."""
        l_format = logging.Formatter(format)
        l_handler = logging.StreamHandler()
        l_handler.setFormatter(l_format)
        self.addHandler(l_handler)

    def set_log_level(
            self,
            log_level):
        """Modifies logger level."""
        self.setLevel(log_level)

    def activate_debug(self):
        """Modifies logger level so it accepts everything equalt or greater than DEBUG."""
        self.set_log_level(logging.DEBUG)


"""Global instance of logger using CustomLogger class."""
logger = CustomLogger()
