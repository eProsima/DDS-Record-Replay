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
"""GUI for the DDS Recorder remote controller."""

from Controller import (
    Controller, ControllerCommand, RecorderStatus,
    MAX_DDS_DOMAIN_ID)

from Logger import logger

from PyQt6.QtCore import QDateTime, QUrl, Qt
from PyQt6.QtGui import QAction, QDesktopServices
from PyQt6.QtWidgets import (
    QDialog, QDialogButtonBox, QHBoxLayout, QHeaderView,
    QLabel, QMainWindow, QMenuBar, QPushButton, QSpinBox, QTableWidget,
    QTableWidgetItem, QVBoxLayout, QWidget)

DDS_RECORDER = 'DDS Recorder'
CONTROLLER = 'Controller'

status_to_color_values__ = {
    RecorderStatus.CLOSED: 'gray',
    RecorderStatus.RUNNING: 'green',
    RecorderStatus.PAUSED: 'yellow',
    RecorderStatus.SUSPENDED: 'pink',
    RecorderStatus.STOPPED: 'red',
    RecorderStatus.UNKNOWN: 'gray'
}


def status_to_color(status: RecorderStatus):
    """Get the color associated with each RecorderStatus."""
    if status in status_to_color_values__:
        return status_to_color_values__[status]
    else:
        return status_to_color_values__[RecorderStatus.UNKNOWN]


def status_to_text(status: RecorderStatus):
    """Convert RecorderStatus enum value to string."""
    return status.name


class DdsDomainDialog(QDialog):
    """Class that implements the a dialog to set the DDS Domain."""

    def __init__(self, current_dds_domain):
        """Construct the dialog to set the DDS Domain."""
        super().__init__()

        self.label = QLabel('Enter DDS Domain:')
        self.spin_box = QSpinBox(self)
        self.spin_box.setMinimum(0)
        self.spin_box.setMaximum(MAX_DDS_DOMAIN_ID)
        self.spin_box.setValue(current_dds_domain)

        self.buttonBox = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Save
            | QDialogButtonBox.StandardButton.Cancel)
        self.buttonBox.accepted.connect(self.accept)
        self.buttonBox.rejected.connect(self.reject)

        layout = QVBoxLayout()
        layout.addWidget(self.label)
        layout.addWidget(self.spin_box)
        layout.addWidget(self.buttonBox)

        self.setLayout(layout)

    def get_dds_domain(self):
        """Return DDS Domain from spin box as integer value."""
        return int(self.spin_box.value())


class MenuWidget(QMenuBar):
    """Class that implements the menu of DDS Recorder controller GUI."""

    def __init__(self, main_window):
        """Construct the Controller GUI menu bar."""
        super().__init__()

        self.main_window = main_window

        file_menu = self.addMenu('File')
        help_menu = self.addMenu('Help')

        dds_domain_action = QAction('DDS Domain', self)
        dds_domain_action.triggered.connect(self.main_window.dds_domain_dialog)
        file_menu.addAction(dds_domain_action)

        docs_action = QAction('Documentation', self)
        docs_action.triggered.connect(
            lambda: QDesktopServices.openUrl(
                QUrl('https://dds-recorder.readthedocs.io/en/latest/')))
        help_menu.addAction(docs_action)

        release_notes_action = QAction('Release Notes', self)
        release_notes_action.triggered.connect(
            lambda: QDesktopServices.openUrl(
                QUrl('https://github.com/eProsima/DDS-Record-Replay/releases')))
        help_menu.addAction(release_notes_action)

        twitter_action = QAction('Join Us on Twitter', self)
        twitter_action.triggered.connect(
            lambda: QDesktopServices.openUrl(
                QUrl('https://twitter.com/EProsima')))
        help_menu.addAction(twitter_action)

        feature_request_action = QAction('Search Feature Requests', self)
        feature_request_action.triggered.connect(
            lambda: QDesktopServices.openUrl(
                QUrl('https://github.com/eProsima/DDS-Record-Replay/releases')))
        help_menu.addAction(feature_request_action)

        issue_action = QAction('Report Issue', self)
        issue_action.triggered.connect(
            lambda: QDesktopServices.openUrl(
                QUrl('https://github.com/eProsima/DDS-Record-Replay/releases/new')))
        help_menu.addAction(issue_action)

        # TODO add something in about
        about_action = QAction('About', self)
        help_menu.addAction(about_action)


class ControllerGUI(QMainWindow):
    """Class that implements the DDS Recorder controller GUI."""

    def __init__(self):
        """Construct the Controller GUI."""
        super().__init__()

        logger.debug('Initializing controller GUI...')

        self.dds_controller = Controller()
        self.dds_domain = 0

        self.init_gui()

        # Connect controller signals to UI functions
        self.dds_controller.on_ddsrecorder_discovered.connect(
            self.on_ddsrecorder_discovered)
        self.dds_controller.on_ddsrecorder_status.connect(
            self.on_ddsrecorder_status)

        # Create DDS entities at the end to avoid race condition (receive
        # status before connecting with signal slots)
        self.dds_controller.init_dds(self.dds_domain)

    def on_ddsrecorder_discovered(self, discovered, message):
        """Inform that a new DDS Recorder has been discovered."""
        if not discovered:
            self.update_status(RecorderStatus.CLOSED)
        self.add_log_entry(CONTROLLER, message)

    def on_ddsrecorder_status(self, previous_status, current_status, info):
        """Inform the status of remote DDS Recorder."""
        # Log entry with new status
        self.add_log_entry(
            DDS_RECORDER,
            f'Update from {previous_status} to {current_status}')

        # Log entry with info
        if info != '':
            self.add_log_entry(DDS_RECORDER, f'Status information: {info}')

        # Change status bar
        self.update_status(RecorderStatus[current_status.upper()])

    def restart_controller(self, dds_domain=0):
        """Restart the DDS Controller if the DDS Domain changes."""
        if dds_domain != self.dds_domain:
            if self.dds_controller.is_valid_dds_domain(dds_domain):
                # Delete DDS entities in previous domain
                self.dds_controller.delete_dds()
                # Reset status
                self.update_status(RecorderStatus.CLOSED)
                # Create DDS entities in new domain
                self.dds_controller.init_dds(dds_domain)
                self.dds_domain = dds_domain

    def init_gui(self):
        """Initialize the graphical interface and its widgets."""
        # Create the menu widget
        menu_widget = MenuWidget(self)
        self.setMenuBar(menu_widget)

        # Vertical layout for the main window
        main_vertical_layout = QVBoxLayout()

        # Status section
        status_box = QHBoxLayout()

        # Label for status bar
        self.label = QLabel(self)
        self.label.setText('eProsima DDS Recorder status:')
        status_box.addWidget(self.label)

        # Status label box
        self.status_box = QLabel(self)
        self.status_box.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.update_status(RecorderStatus.CLOSED)

        status_box.addWidget(self.status_box, 1)

        main_vertical_layout.addLayout(status_box)

        # Horizontal layout for buttons and logs
        buttons_logs_box = QHBoxLayout()

        # Horizontal layout for the buttons
        buttons_box = QVBoxLayout()

        # Add a button per command
        start_button = QPushButton('Start', self)
        start_button.clicked.connect(
            lambda: self.simple_button_clicked(
                ControllerCommand.start))
        buttons_box.addWidget(start_button)

        pause_button = QPushButton('Pause', self)
        pause_button.clicked.connect(
            lambda: self.simple_button_clicked(
                ControllerCommand.pause))
        buttons_box.addWidget(pause_button)

        suspend_button = QPushButton('Suspend', self)
        suspend_button.clicked.connect(
            lambda: self.simple_button_clicked(
                ControllerCommand.suspend))
        buttons_box.addWidget(suspend_button)

        stop_button = QPushButton('Stop', self)
        stop_button.clicked.connect(
            lambda: self.simple_button_clicked(
                ControllerCommand.stop))
        buttons_box.addWidget(stop_button)

        event_button = QPushButton('Event', self)
        event_button.clicked.connect(
            lambda: self.simple_button_clicked(
                ControllerCommand.event))
        buttons_box.addWidget(event_button)

        event_start_button = QPushButton('Event && Start', self)
        event_start_button.clicked.connect(
            lambda: self.event_start_button_clicked())
        buttons_box.addWidget(event_start_button)

        event_suspend_button = QPushButton('Event && Suspend', self)
        event_suspend_button.clicked.connect(
            lambda: self.event_suspend_button_clicked())
        buttons_box.addWidget(event_suspend_button)

        event_stop_button = QPushButton('Event && Stop', self)
        event_stop_button.clicked.connect(
            lambda: self.event_stop_button_clicked())
        buttons_box.addWidget(event_stop_button)

        close_button = QPushButton('Close', self)
        close_button.clicked.connect(
            lambda: self.simple_button_clicked(
                ControllerCommand.close))
        buttons_box.addWidget(close_button)
        buttons_box.addStretch()

        # Add the horizontal layout to the vertical layout
        buttons_logs_box.addLayout(buttons_box)

        # Create a text box for displaying the log
        self.log_box = QTableWidget(self)
        self.log_box.setColumnCount(3)
        self.log_box.setHorizontalHeaderLabels(
            ['Timestamp', 'Source', 'Message'])
        header = self.log_box.horizontalHeader()
        header.resizeSection(0, 200)
        header.resizeSection(1, 100)
        header.setSectionResizeMode(2, QHeaderView.ResizeMode.Stretch)
        buttons_logs_box.addWidget(self.log_box)

        main_vertical_layout.addLayout(buttons_logs_box)

        # Add the layout to a widget and set it as the central
        # widget of the window
        central_widget = QWidget()
        central_widget.setLayout(main_vertical_layout)
        self.setCentralWidget(central_widget)

        # Set the window title and show the window
        self.setWindowTitle('DDS Recorder controller')
        self.resize(700, 400)
        self.show()

    def update_status(self, status: RecorderStatus):
        """Update status box."""
        self.status_box.setText(status_to_text(status))
        self.status_box.setStyleSheet(
            'background-color: '
            f'{status_to_color(status)}')

    def dds_domain_dialog(self):
        """Create a dialog to update the DDS Domain."""
        dialog = DdsDomainDialog(self.dds_domain)
        if dialog.exec():
            domain = dialog.get_dds_domain()
            # Restart only if it is a different Domain
            if domain != self.dds_domain:
                if (self.dds_controller.is_valid_dds_domain(domain)):
                    self.restart_controller(dds_domain=domain)

    def event_start_button_clicked(self):
        """Publish command."""
        command = ControllerCommand.event
        args = Controller.command_arguments_to_string(
            Controller.argument_change_state(RecorderStatus.RUNNING))
        self.send_command(command, args)

    def event_suspend_button_clicked(self):
        """Publish command."""
        command = ControllerCommand.event
        args = Controller.command_arguments_to_string(
            Controller.argument_change_state(RecorderStatus.SUSPENDED))
        self.send_command(command, args)

    def event_stop_button_clicked(self):
        """Publish command."""
        command = ControllerCommand.event
        args = Controller.command_arguments_to_string(
            Controller.argument_change_state(RecorderStatus.STOPPED))
        self.send_command(command, args)

    def simple_button_clicked(self, command):
        """Publish command."""
        self.send_command(command)

    def send_command(self, command, args=''):
        """Publish command."""
        if (self.dds_controller.publish_command(command, args)):
            self.add_log_entry(
                CONTROLLER, f'{command.name} [{args}] command sent')

    def add_log_entry(self, source, message):
        """Add log entry to the logging table."""
        # Get the current datetime and format it
        timestamp = QDateTime.currentDateTime().toString(
            'yyyy-MM-dd hh:mm:ss,z')

        # Insert a new row in the table with timestamp, source, and message
        row_count = self.log_box.rowCount()
        self.log_box.insertRow(row_count)

        self.log_box.setItem(
            row_count, 0, CustomNonEditableQTableWidgetItem(timestamp))
        self.log_box.setItem(
            row_count, 1, CustomNonEditableQTableWidgetItem(source))
        self.log_box.setItem(
            row_count, 2, CustomNonEditableQTableWidgetItem(message))

    def closeEvent(self, event):
        """Update close event to close the application properly."""
        # Call the parent closeEvent to close the application
        super().closeEvent(event)


class CustomNonEditableQTableWidgetItem(QTableWidgetItem):
    """Class that implements a non editable QTableWidgetItem."""

    def __init__(self, arg):
        """Construct a CustomNonEditableQTableWidgetItem."""
        super().__init__(arg)
        self.setFlags(self.flags() & ~Qt.ItemFlag.ItemIsEditable)
