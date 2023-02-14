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
"""Script implementing a remote controller for the DDS Recorder."""

import logging

import fastdds

from type.ControllerCommand import ControllerCommand


class WriterListener (fastdds.DataWriterListener):
    def __init__(self, writer):
        self._writer = writer
        super().__init__()

    def on_publication_matched(self, datawriter, info):
        if (0 < info.current_count_change):
            print('DDS Recorder found!')
        else:
            print('DDS Recorder lost!')


class Controller(object):
    """
    TODO
    """

    def __init__(
        self,
        debug=False,
        logger=None
    ):
        """
        TODO
        """
        self.set_logger(logger, debug)
        self.logger.debug('Initializing DDS Recorder controller...')

        # Initialize DDS entities
        factory = fastdds.DomainParticipantFactory.get_instance()
        participant_qos = fastdds.DomainParticipantQos()
        factory.get_default_participant_qos(participant_qos)
        self.participant = factory.create_participant(0, participant_qos)

        self.topic_data_type = ControllerCommand.ControllerCommandPubSubType()
        self.topic_data_type.setName('ControllerCommand')
        type_support = fastdds.TypeSupport(self.topic_data_type)
        self.participant.register_type(type_support)

        topic_qos = fastdds.TopicQos()
        self.participant.get_default_topic_qos(topic_qos)
        self.topic = self.participant.create_topic(
            'ddsrecorder/controller',
            self.topic_data_type.getName(),
            topic_qos)

        publisher_qos = fastdds.PublisherQos()
        self.participant.get_default_publisher_qos(publisher_qos)
        self.publisher = self.participant.create_publisher(publisher_qos)

        self.listener = WriterListener(self)
        writer_qos = fastdds.DataWriterQos()
        self.publisher.get_default_datawriter_qos(writer_qos)
        self.writer = self.publisher.create_datawriter(
            self.topic, writer_qos, self.listener)


    def set_logger(self, logger, debug):
        """
        Instance the class logger.

        :param logger: The logging object. CONTROLLER if None
            logger is provided.
        :param debug: True/False to activate/deactivate debug logger.
        """
        if isinstance(logger, logging.Logger):
            self.logger = logger
        else:
            if isinstance(logger, str):
                self.logger = logging.getLogger(logger)
            else:
                self.logger = logging.getLogger('CONTROLLER')

            if not self.logger.hasHandlers():
                l_handler = logging.StreamHandler()
                l_format = '[%(asctime)s][%(name)s][%(levelname)s] %(message)s'
                l_format = logging.Formatter(l_format)
                l_handler.setFormatter(l_format)
                self.logger.addHandler(l_handler)

        if debug:
            self.logger.setLevel(logging.DEBUG)
        else:
            self.logger.setLevel(logging.INFO)


