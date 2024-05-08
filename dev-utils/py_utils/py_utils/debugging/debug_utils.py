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
This file contains utils to debug python code.
"""

from py_utils.logging.log_utils import logger


def debug_object_introspection(obj):
    """Log in debug introspection information regarding an object."""
    class_name = type(obj).__name__
    methods = [method_name for method_name in dir(obj) if callable(getattr(obj, method_name))]
    logger.debug(f'{{Class: <{class_name}; Methods: {methods}}}')


def debug_function_decorator(
        func):
    """Decorator to debug information regarding start, arguments and finish of a function."""
    def wrapper(*args, **kwargs):
        logger.debug(f'Function <{func.__name__}> called with arguments: <{args}>, <{kwargs}>')
        result = func(*args, **kwargs)
        logger.debug(f'Function <{func.__name__}> finished')
        return result

    return wrapper
