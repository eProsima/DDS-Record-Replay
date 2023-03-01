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

import logging


class CustomLogger(logging.Logger):

    def __init__(
            self,
            log_level=logging.INFO):
        super().__init__('GLOBAL_LOG')
        self.set_format()
        self.set_log_level(log_level)

    def set_format(
            self,
            format: str = '[%(asctime)s][%(name)s][%(levelname)s] %(message)s'):
        l_format = logging.Formatter(format)
        l_handler = logging.StreamHandler()
        l_handler.setFormatter(l_format)
        self.addHandler(l_handler)

    def set_log_level(
            self,
            log_level):
        self.setLevel(log_level)

    def activate_debug(self):
        self.set_log_level(logging.DEBUG)


# Global instance of logger
logger = CustomLogger()
