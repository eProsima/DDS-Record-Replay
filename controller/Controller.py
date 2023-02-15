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

from ddstypes.ControllerCommand.ControllerCommand import (
    ControllerCommand, ControllerCommandPubSubType)

import fastdds

from utils.Logger import Logger


class WriterListener (fastdds.DataWriterListener):
    """Fast DDS DataWriter listener class."""

    def __init__(self, writer):
        """Construct a WriterListener object."""
        self._writer = writer
        super().__init__()

    def on_publication_matched(self, datawriter, info):
        """Raise when there has been a match/unmatch event."""
        if (0 < info.current_count_change):
            self.logger.debug('DDS Recorder found!')
        else:
            self.logger.debug('DDS Recorder lost!')


class Controller(object):
    """DDS Controller class."""

    def __init__(
        self,
        dds_domain=0,
        debug=False,
        logger=None
    ):
        """Construct a DDS Controller."""
        # Initialize logger
        self.logger = Logger(debug, logger)
        self.logger.debug('Initializing DDS Recorder controller...')

        # Check DDS Domain
        if (not self.is_valid_dds_domain(dds_domain)):
            raise ValueError(
                'DDS Domain must be a number between 0 and 255.')

        self.init_dds(dds_domain)

    def __del__(self):
        """Remove all dds entities in an orderly manner."""
        self.delete()

    def is_valid_dds_domain(self, dds_domain):
        return ((dds_domain >= 0) and (dds_domain <= 255))

    def init_dds(self, dds_domain):
        """Initialize DDS entities."""
        factory = fastdds.DomainParticipantFactory.get_instance()
        participant_qos = fastdds.DomainParticipantQos()
        factory.get_default_participant_qos(participant_qos)
        participant_qos.name('DDSRecorderController')
        self.participant = factory.create_participant(
            dds_domain, participant_qos)

        self.topic_data_type = ControllerCommandPubSubType()
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
        writer_qos.reliability().kind = fastdds.RELIABLE_RELIABILITY_QOS
        writer_qos.durability().kind = fastdds.TRANSIENT_LOCAL_DURABILITY_QOS
        writer_qos.history().kind = fastdds.KEEP_LAST_HISTORY_QOS
        writer_qos.history().size = 10
        self.writer = self.publisher.create_datawriter(
            self.topic, writer_qos, self.listener)

        subscriber_qos = fastdds.SubscriberQos()
        self.participant.get_default_subscriber_qos(subscriber_qos)
        self.subscriber = self.participant.create_subscriber(subscriber_qos)

    def publish_command(self, command, args=''):
        """
        Publish a command.

        :param command: The command to be published.
        :param debug: The arguments of the command to be published.
        """
        data = ControllerCommand()
        data.command(command)
        data.args(args)
        if(not self.writer.write(data)):
            self.logger.error('Publish failed')

    def start(self):
        """Publish START command."""
        self.publish_command('START')

    def pause(self):
        """Publish PAUSE command."""
        self.publish_command('PAUSE')

    def stop(self):
        """Publish STOP command."""
        self.publish_command('STOP')

    def event(self):
        """Publish EVENT command."""
        self.publish_command('EVENT')

    def close(self):
        """Publish CLOSE command."""
        self.publish_command('CLOSE')

    def delete(self):
        factory = fastdds.DomainParticipantFactory.get_instance()
        self.participant.delete_contained_entities()
        factory.delete_participant(self.participant)
