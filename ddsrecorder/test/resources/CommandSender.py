# exec(open('../../../CommandSender.py').read())
# sender = CommandSender(100)
# sender.send("start")

import fastdds
import DdsRecorderCommand

class WriterListener (fastdds.DataWriterListener):
    def __init__(self, writer):
        self._writer = writer
        super().__init__()

    def on_publication_matched(self, datawriter, info):
        if (0 < info.current_count_change):
            print("Publisher matched subscriber {}".format(info.last_subscription_handle))
        else:
            print ("Publisher unmatched subscriber {}".format(info.last_subscription_handle))

class CommandSender:
    def __init__(self, domain):
        factory = fastdds.DomainParticipantFactory.get_instance()
        self.participant_qos = fastdds.DomainParticipantQos()
        factory.get_default_participant_qos(self.participant_qos)
        self.participant = factory.create_participant(domain, self.participant_qos)

        self.topic_data_type = DdsRecorderCommand.DdsRecorderCommandPubSubType()
        self.topic_data_type.setName("DdsRecorderCommand")
        self.type_support = fastdds.TypeSupport(self.topic_data_type)
        self.participant.register_type(self.type_support)

        self.topic_qos = fastdds.TopicQos()
        self.participant.get_default_topic_qos(self.topic_qos)
        self.topic = self.participant.create_topic("/ddsrecorder/command", self.topic_data_type.getName(), self.topic_qos)

        self.publisher_qos = fastdds.PublisherQos()
        self.participant.get_default_publisher_qos(self.publisher_qos)
        self.publisher = self.participant.create_publisher(self.publisher_qos)

        self.listener = WriterListener(self)
        self.writer_qos = fastdds.DataWriterQos()
        self.publisher.get_default_datawriter_qos(self.writer_qos)
        self.writer = self.publisher.create_datawriter(self.topic, self.writer_qos, self.listener)

    def send(self, command):
        data = DdsRecorderCommand.DdsRecorderCommand()
        data.command(command)
        self.writer.write(data)
        print("Sending command {}".format(command))

    def __del__(self):
        factory = fastdds.DomainParticipantFactory.get_instance()
        self.participant.delete_contained_entities()
        factory.delete_participant(self.participant)
