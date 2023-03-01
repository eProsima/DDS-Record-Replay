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

from ControllerCommand import ControllerCommand, ControllerCommandPubSubType

from Logger import logger

from PyQt6.QtCore import QObject, pyqtSignal

from Status import Status, StatusPubSubType

import fastdds


MAX_DDS_DOMAIN_ID = 232


class DdsRecorderStatus(Enum):
    """Possible Status values from a DDS Recorder."""

    CLOSED = 0
    RUNNING = 1
    PAUSED = 2
    STOPPED = 3
    UNKNOWN = 4


class DdsRecorderControllerCommand(Enum):
    """
    Possible commands from a Controller to a DDS Recorder.

    Each command is represented by a string (same as enum name).
    Commands can have arguments in a format of a json with name of argument
    and value in strings.
    """

    START = 0
    STOP = 1
    PAUSE = 2
    EVENT = 3
    CLOSE = 4


class CommandWriterListener(fastdds.DataWriterListener):
    """Fast DDS DataWriter listener class for ControllerCommand DataWriter."""

    def __init__(self, controller):
        """Construct a WriterListener object."""
        self._controller = controller
        super().__init__()

    def on_publication_matched(self, writer, info):
        """Raise when there has been a match/unmatch event."""
        if (0 < info.current_count_change):
            logger.debug('DDS Recorder found!')
            self._controller.on_ddsrecorder_discovered.emit(
                True, 'DDS Recorder found!')
        else:
            logger.debug('DDS Recorder lost!')
            self._controller.on_ddsrecorder_discovered.emit(
                False, 'DDS Recorder lost!')


class StatusReaderListener(fastdds.DataReaderListener):
    """Fast DDS DataReader listener class for Status DataReader."""

    def __init__(self, controller):
        """Construct a WriterListener object."""
        self._controller = controller
        super().__init__()

    def on_data_available(self, reader):
        """Raise when there is new Status data available."""
        info = fastdds.SampleInfo()
        data = Status()
        reader.take_next_sample(data, info)
        self._controller.on_ddsrecorder_status.emit(
            data.previous(), data.current(), data.info())


class Controller(QObject):
    """DDS Controller class."""

    def __init__(
        self,
        dds_domain=0,
        debug=False
    ):
        """Construct a DDS Controller."""
        super().__init__()

        logger.debug('Initializing DDS Recorder controller...')

        # Check DDS Domain
        if (not self.is_valid_dds_domain(dds_domain)):
            raise ValueError(
                f'DDS Domain must be a number '
                f'between 0 and {MAX_DDS_DOMAIN_ID}.')

        self.init_dds(dds_domain)

    def __del__(self):
        """Remove all dds entities in an orderly manner."""
        self.delete()

    on_ddsrecorder_discovered = pyqtSignal(bool, str)
    on_ddsrecorder_status = pyqtSignal(str, str, str)

    def is_valid_dds_domain(self, dds_domain):
        """Check if DDS Domain is valid."""
        return ((dds_domain >= 0) and (dds_domain <= MAX_DDS_DOMAIN_ID))

    def init_dds(self, dds_domain):
        """Initialize DDS entities."""
        factory = fastdds.DomainParticipantFactory.get_instance()
        self.participant_qos = fastdds.DomainParticipantQos()
        factory.get_default_participant_qos(self.participant_qos)
        self.participant_qos.name('DDSRecorderController')
        self.participant = factory.create_participant(
            dds_domain, self.participant_qos)

        # Initialize command topic
        self.command_topic_data_type = ControllerCommandPubSubType()
        self.command_topic_data_type.setName('ControllerCommand')
        self.command_type_support = fastdds.TypeSupport(self.command_topic_data_type)
        self.participant.register_type(self.command_type_support)

        self.command_topic_qos = fastdds.TopicQos()
        self.participant.get_default_topic_qos(self.command_topic_qos)
        self.command_topic = self.participant.create_topic(
            '/ddsrecorder/command',
            self.command_topic_data_type.getName(),
            self.command_topic_qos)

        # Initialize status topic
        self.status_topic_data_type = StatusPubSubType()
        self.status_topic_data_type.setName('Status')
        self.status_type_support = fastdds.TypeSupport(self.status_topic_data_type)
        self.participant.register_type(self.status_type_support)

        self.status_topic_qos = fastdds.TopicQos()
        self.participant.get_default_topic_qos(self.status_topic_qos)
        self.status_topic = self.participant.create_topic(
            '/ddsrecorder/status',
            self.status_topic_data_type.getName(),
            self.status_topic_qos)

        # Initialize Command writer
        self.publisher_qos = fastdds.PublisherQos()
        self.participant.get_default_publisher_qos(self.publisher_qos)
        self.publisher = self.participant.create_publisher(self.publisher_qos)

        self.command_writer_listener = CommandWriterListener(self)
        self.writer_qos = fastdds.DataWriterQos()
        self.publisher.get_default_datawriter_qos(self.writer_qos)
        self.writer_qos.reliability().kind = fastdds.RELIABLE_RELIABILITY_QOS
        self.writer_qos.durability().kind = fastdds.VOLATILE_DURABILITY_QOS
        self.writer_qos.history().kind = fastdds.KEEP_LAST_HISTORY_QOS
        self.writer_qos.history().size = 10
        self.command_writer = self.publisher.create_datawriter(
            self.command_topic, self.writer_qos, self.command_writer_listener)

        # Initialize Status reader
        self.subscriber_qos = fastdds.SubscriberQos()
        self.participant.get_default_subscriber_qos(self.subscriber_qos)
        self.subscriber = self.participant.create_subscriber(self.subscriber_qos)

        self.status_reader_listener = StatusReaderListener(self)
        self.reader_qos = fastdds.DataReaderQos()
        self.subscriber.get_default_datareader_qos(self.reader_qos)
        self.reader_qos.reliability().kind = fastdds.RELIABLE_RELIABILITY_QOS
        self.reader_qos.durability().kind = fastdds.TRANSIENT_LOCAL_DURABILITY_QOS
        self.reader_qos.history().kind = fastdds.KEEP_LAST_HISTORY_QOS
        self.reader_qos.history().size = 10
        self.status_reader = self.subscriber.create_datareader(
            self.status_topic, self.reader_qos, self.status_reader_listener)

    def publish_command(
            self,
            command: DdsRecorderControllerCommand,
            args: str = ''):
        """
        Publish a command.

        :param command: The command to be published.
        :param debug: The arguments of the command to be published.
        """
        data = ControllerCommand()
        data.command(command.name)
        data.args(args)
        if (not self.command_writer.write(data)):
            logger.error('Publish failed')
            return False

        return True

    def delete(self):
        """Delete DDS instances."""
        factory = fastdds.DomainParticipantFactory.get_instance()
        self.participant.delete_contained_entities()
        factory.delete_participant(self.participant)

    def command_arguments_to_string(args: dict):
        """Serialize json object to string."""
        # TODO convert to json string
        return str(args)

    def argument_change_state(next_state: DdsRecorderStatus):
        """Set next state EVENT command argument."""
        return {'next_state': f'{next_state.name}'}
