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

from enum import Enum

from PyQt6.QtCore import QObject, pyqtSignal

from ddstypes.ControllerCommand.ControllerCommand import (
    ControllerCommand, ControllerCommandPubSubType)
from ddstypes.Status.Status import Status, StatusPubSubType

import fastdds

from utils.Logger import Logger


class DdsRecorderStatus(Enum):
    RECORDING = 1
    PAUSED = 2
    STOPPED = 3
    UNKNOWN = 4


class CommandWriterListener(fastdds.DataWriterListener):
    """Fast DDS DataWriter listener class for ControllerCommand DataWriter."""

    def __init__(self, controller):
        """Construct a WriterListener object."""
        super().__init__()
        self._controller = controller

    def on_publication_matched(self, writer, info):
        """Raise when there has been a match/unmatch event."""
        if (0 < info.current_count_change):
            self.logger.debug('DDS Recorder found!')
            self._controller.on_ddsrecorder_discovery.emit(
                'DDS Recorder found!')
        else:
            self.logger.debug('DDS Recorder lost!')
            self._controller.on_ddsrecorder_discovery.emit(
                'DDS Recorder lost!')


class StatusReaderListener(fastdds.DataReaderListener):
    """Fast DDS DataReader listener class for Status DataReader."""

    def __init__(self, controller):
        """Construct a WriterListener object."""
        super().__init__()
        self._controller = controller

    def on_data_available(self, reader):
        """Raise when there is new Status data available."""
        info = fastdds.SampleInfo()
        data = Status()
        reader.take_next_sample(data, info)
        self._controller.on_ddsrecorder_status.emit(
            data.previous, data.current, data.info)


class Controller(QObject):
    """DDS Controller class."""

    def __init__(
        self,
        dds_domain=0,
        debug=False,
        logger=None
    ):
        """Construct a DDS Controller."""
        super().__init__()
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

    on_ddsrecorder_discovered = pyqtSignal(str)
    on_ddsrecorder_status = pyqtSignal(str, str, str)

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

        # Initialize command topic
        self.command_topic_data_type = ControllerCommandPubSubType()
        self.command_topic_data_type.setName('ControllerCommand')
        type_support = fastdds.TypeSupport(self.command_topic_data_type)
        self.participant.register_type(type_support)

        topic_qos = fastdds.TopicQos()
        self.participant.get_default_topic_qos(topic_qos)
        self.command_topic = self.participant.create_topic(
            'ddsrecorder/controller',
            self.command_topic_data_type.getName(),
            topic_qos)

        # Initialize status topic
        self.status_topic_data_type = StatusPubSubType()
        self.status_topic_data_type.setName('Status')
        type_support = fastdds.TypeSupport(self.status_topic_data_type)
        self.participant.register_type(type_support)

        topic_qos = fastdds.TopicQos()
        self.participant.get_default_topic_qos(topic_qos)
        self.status_topic = self.participant.create_topic(
            'ddsrecorder/status',
            self.status_topic_data_type.getName(),
            topic_qos)

        publisher_qos = fastdds.PublisherQos()
        self.participant.get_default_publisher_qos(publisher_qos)
        self.publisher = self.participant.create_publisher(publisher_qos)

        self.command_writer_listener = CommandWriterListener(self)
        writer_qos = fastdds.DataWriterQos()
        self.publisher.get_default_datawriter_qos(writer_qos)
        writer_qos.reliability().kind = fastdds.RELIABLE_RELIABILITY_QOS
        writer_qos.durability().kind = fastdds.TRANSIENT_LOCAL_DURABILITY_QOS
        writer_qos.history().kind = fastdds.KEEP_LAST_HISTORY_QOS
        writer_qos.history().size = 10
        self.command_writer = self.publisher.create_datawriter(
            self.command_topic, writer_qos, self.command_writer_listener)

        subscriber_qos = fastdds.SubscriberQos()
        self.participant.get_default_subscriber_qos(subscriber_qos)
        self.subscriber = self.participant.create_subscriber(subscriber_qos)

        self.status_reader_listener = StatusReaderListener(self)
        reader_qos = fastdds.DataReaderQos()
        self.subscriber.get_default_datareader_qos(reader_qos)
        reader_qos.reliability().kind = fastdds.RELIABLE_RELIABILITY_QOS
        reader_qos.durability().kind = fastdds.TRANSIENT_LOCAL_DURABILITY_QOS
        reader_qos.history().kind = fastdds.KEEP_LAST_HISTORY_QOS
        reader_qos.history().size = 1
        self.status_reader = self.subscriber.create_datareader(
            self.status_topic, reader_qos, self.status_reader_listener)

    def publish_command(self, command, args=''):
        """
        Publish a command.

        :param command: The command to be published.
        :param debug: The arguments of the command to be published.
        """
        data = ControllerCommand()
        data.command(command)
        data.args(args)
        if(not self.command_writer.write(data)):
            self.logger.error('Publish failed')
            return False

        return True

    def start(self):
        """Publish START command."""
        return self.publish_command('START')

    def pause(self):
        """Publish PAUSE command."""
        return self.publish_command('PAUSE')

    def stop(self):
        """Publish STOP command."""
        return self.publish_command('STOP')

    def event(self):
        """Publish EVENT command."""
        return self.publish_command('EVENT')

    def close(self):
        """Publish CLOSE command."""
        return self.publish_command('CLOSE')

    def delete(self):
        factory = fastdds.DomainParticipantFactory.get_instance()
        self.participant.delete_contained_entities()
        factory.delete_participant(self.participant)
