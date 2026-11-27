.. add orphan tag when new info added to this file

:orphan:

###################
Forthcoming Version
###################

Next release will include the following **features**:

* Add the ``endpoint-qos-mode`` tag to choose how a matching XML endpoint profile is applied
  (see :ref:`DDS Recorder <recorder_usage_configuration_endpoint_qos_mode>` and
  :ref:`DDS Replayer <replayer_usage_configuration_endpoint_qos_mode>`).
  By default (``xml-overridable``), the profile is used as base and its durability, reliability, ownership and history
  are taken from the YAML configuration, or from the discovered DataWriters (*DDS Recorder*) and the input file
  (*DDS Replayer*); ``xml-standalone`` applies the profile as is; ``disabled`` does not look up any XML endpoint
  profile.
