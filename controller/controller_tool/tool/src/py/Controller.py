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

import json
import os
if os.name == 'nt':
    import win32api

from enum import Enum

if os.name == 'nt':
    win32api.LoadLibrary('DdsRecorderCommand')
from DdsRecorderCommand import DdsRecorderCommand, DdsRecorderCommandPubSubType

from Logger import logger

from PyQt6.QtCore import QObject, pyqtSignal

if os.name == 'nt':
    win32api.LoadLibrary('DdsRecorderStatus')
from DdsRecorderStatus import DdsRecorderStatus, DdsRecorderStatusPubSubType

import fastdds


MAX_DDS_DOMAIN_ID = 232


class RecorderStatus(Enum):
    """Possible Status values from a DDS Recorder."""

    CLOSED = 0
    RUNNING = 1
    PAUSED = 2
    SUSPENDED = 3
    STOPPED = 4
    UNKNOWN = 5


class ControllerCommand(Enum):
    """
    Possible commands from a Controller to a DDS Recorder.

    Each command is represented by a string (same as enum name).
    Commands can have arguments in a format of a json with name of argument
    and value in strings.
    """

    start = 0
    stop = 1
    suspend = 2
    pause = 3
    event = 4
    close = 5


class CommandWriterListener(fastdds.DataWriterListener):
    """Fast DDS DataWriter listener class for DdsRecorderCommand DataWriter."""

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
        data = DdsRecorderStatus()
        reader.take_next_sample(data, info)
        self._controller.on_ddsrecorder_status.emit(
            data.previous(), data.current(), data.info())


class Controller(QObject):
    """DDS Controller class."""

    def __init__(
        self,
        debug=False
    ):
        """Construct a DDS Controller."""
        super().__init__()

        logger.debug('Initializing DDS Recorder controller...')

    def __del__(self):
        """Remove all dds entities in an orderly manner."""
        self.delete_dds()
        super().__del_()

    on_ddsrecorder_discovered = pyqtSignal(bool, str)
    on_ddsrecorder_status = pyqtSignal(str, str, str)

    def is_valid_dds_domain(self, dds_domain):
        """Check if DDS Domain is valid."""
        return ((dds_domain >= 0) and (dds_domain <= MAX_DDS_DOMAIN_ID))

    def init_dds(self, dds_domain, command_topic, status_topic):
        # Check DDS Domain
        if (not self.is_valid_dds_domain(dds_domain)):
            raise ValueError(
                f'DDS Domain must be a number '
                f'between 0 and {MAX_DDS_DOMAIN_ID}.')

        """Initialize DDS entities."""
        factory = fastdds.DomainParticipantFactory.get_instance()
        participant_qos = fastdds.DomainParticipantQos()
        factory.get_default_participant_qos(participant_qos)
        participant_qos.name('DDSRecorderController')
        self.participant = factory.create_participant(
            dds_domain, participant_qos)

        # Initialize command topic
        self.command_topic_data_type = DdsRecorderCommandPubSubType()
        self.command_topic_data_type.setName('DdsRecorderCommand')
        self.command_type_support = fastdds.TypeSupport(self.command_topic_data_type)
        self.participant.register_type(self.command_type_support)

        command_topic_qos = fastdds.TopicQos()
        self.participant.get_default_topic_qos(command_topic_qos)
        self.command_topic = self.participant.create_topic(
            command_topic,
            self.command_topic_data_type.getName(),
            command_topic_qos)

        # Initialize status topic
        self.status_topic_data_type = DdsRecorderStatusPubSubType()
        self.status_topic_data_type.setName('DdsRecorderStatus')
        self.status_type_support = fastdds.TypeSupport(self.status_topic_data_type)
        self.participant.register_type(self.status_type_support)

        status_topic_qos = fastdds.TopicQos()
        self.participant.get_default_topic_qos(status_topic_qos)
        self.status_topic = self.participant.create_topic(
            status_topic,
            self.status_topic_data_type.getName(),
            status_topic_qos)

        # Initialize Command writer
        publisher_qos = fastdds.PublisherQos()
        self.participant.get_default_publisher_qos(publisher_qos)
        self.publisher = self.participant.create_publisher(publisher_qos)

        self.command_writer_listener = CommandWriterListener(self)
        writer_qos = fastdds.DataWriterQos()
        self.publisher.get_default_datawriter_qos(writer_qos)
        writer_qos.reliability().kind = fastdds.RELIABLE_RELIABILITY_QOS
        writer_qos.durability().kind = fastdds.VOLATILE_DURABILITY_QOS
        writer_qos.history().kind = fastdds.KEEP_LAST_HISTORY_QOS
        writer_qos.history().size = 10
        self.command_writer = self.publisher.create_datawriter(
            self.command_topic, writer_qos, self.command_writer_listener)

        # Initialize Status reader
        self.subscriber_qos = fastdds.SubscriberQos()
        self.participant.get_default_subscriber_qos(self.subscriber_qos)
        self.subscriber = self.participant.create_subscriber(self.subscriber_qos)

        self.status_reader_listener = StatusReaderListener(self)
        reader_qos = fastdds.DataReaderQos()
        self.subscriber.get_default_datareader_qos(reader_qos)
        reader_qos.reliability().kind = fastdds.RELIABLE_RELIABILITY_QOS
        reader_qos.durability().kind = fastdds.TRANSIENT_LOCAL_DURABILITY_QOS
        reader_qos.history().kind = fastdds.KEEP_LAST_HISTORY_QOS
        reader_qos.history().size = 10
        self.status_reader = self.subscriber.create_datareader(
            self.status_topic, reader_qos, self.status_reader_listener)

    def publish_command(
            self,
            command: ControllerCommand,
            args: str = ''):
        """
        Publish a command.

        :param command: The command to be published.
        :param debug: The arguments of the command to be published.
        """
        data = DdsRecorderCommand()
        data.command(command.name)
        data.args(args)
        if (not self.command_writer.write(data)):
            logger.error('Publish failed')
            return False

        return True

    def delete_dds(self):
        """Delete DDS instances."""
        factory = fastdds.DomainParticipantFactory.get_instance()
        self.participant.delete_contained_entities()
        factory.delete_participant(self.participant)

    def command_arguments_to_string(args: dict):
        """Serialize json object to string."""
        return json.dumps(args)

    def argument_change_state(next_state: DdsRecorderStatus):
        """Set next state EVENT command argument."""
        return {'next_state': f'{next_state.name}'}
