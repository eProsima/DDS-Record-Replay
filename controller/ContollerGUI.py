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

import sys

from Controller import Controller

from PyQt6.QtCore import QDateTime, Qt, QUrl
from PyQt6.QtGui import QAction, QDesktopServices
from PyQt6.QtWidgets import (
    QApplication, QDialog, QDialogButtonBox, QHBoxLayout, QHeaderView,
    QLabel, QMainWindow, QMenuBar, QPushButton, QSpinBox, QTableWidget,
    QTableWidgetItem, QVBoxLayout, QWidget)

from utils.Logger import Logger


class DdsDomainDialog(QDialog):
    """Class that implements the a dialog to set the DDS Domain."""

    def __init__(self, current_dds_domain):
        """Construct the dialog to set the DDS Domain."""
        super().__init__()

        self.label = QLabel('Enter DDS Domain:')
        self.spin_box = QSpinBox(self)
        self.spin_box.setMinimum(0)
        self.spin_box.setMaximum(255)
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
                QUrl("https://dds-recorder.readthedocs.io/en/latest/")))
        help_menu.addAction(docs_action)

        release_notes_action = QAction('Release Notes', self)
        release_notes_action.triggered.connect(
            lambda: QDesktopServices.openUrl(
                QUrl("https://github.com/eProsima/DDS-Recorder/releases")))
        help_menu.addAction(release_notes_action)

        twitter_action = QAction('Join Us on Twitter', self)
        twitter_action.triggered.connect(
            lambda: QDesktopServices.openUrl(
                QUrl("https://twitter.com/EProsima")))
        help_menu.addAction(twitter_action)

        feature_request_action = QAction('Search Feature Requests', self)
        feature_request_action.triggered.connect(
            lambda: QDesktopServices.openUrl(
                QUrl("https://github.com/eProsima/DDS-Recorder/releases")))
        help_menu.addAction(feature_request_action)

        issue_action = QAction('Report Issue', self)
        issue_action.triggered.connect(
            lambda: QDesktopServices.openUrl(
                QUrl("https://github.com/eProsima/DDS-Recorder/releases/new")))
        help_menu.addAction(issue_action)

        about_action = QAction('About', self)
        help_menu.addAction(about_action)


class ControllerGUI(QMainWindow):
    """Class that implements the DDS Recorder controller GUI."""

    def __init__(self):
        """Construct the Controller GUI."""
        super().__init__()

        self.dds_controller = Controller()
        self.logger = Logger()
        self.logger.debug('Initializing controller GUI...')
        self.init_gui()
        self.dds_domain = 0

    def restart_controller(self, dds_domain=0):
        """Restart the DDS Controller if the DDS Domain changes."""
        if (dds_domain != self.dds_domain):
            self.dds_controller.delete()
            self.dds_controller = Controller(
                dds_domain=dds_domain)
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
        self.status_box.setStyleSheet('background-color: gray')
        status_box.addWidget(self.status_box, 1)

        main_vertical_layout.addLayout(status_box)

        # Horizontal layout for the buttons
        buttons_box = QHBoxLayout()

        start_button = QPushButton('Start', self)
        start_button.clicked.connect(self.start_clicked)
        buttons_box.addWidget(start_button)

        pause_button = QPushButton('Pause', self)
        pause_button.clicked.connect(self.pause_clicked)
        buttons_box.addWidget(pause_button)

        stop_button = QPushButton('Stop', self)
        stop_button.clicked.connect(self.stop_clicked)
        buttons_box.addWidget(stop_button)

        event_button = QPushButton('Event', self)
        event_button.clicked.connect(self.event_clicked)
        buttons_box.addWidget(event_button)

        close_button = QPushButton('Close', self)
        close_button.clicked.connect(self.close_clicked)
        buttons_box.addWidget(close_button)

        # Add the horizontal layout to the vertical layout
        main_vertical_layout.addLayout(buttons_box)

        # Create a text box for displaying the log
        self.log_box = QTableWidget(self)
        self.log_box.setColumnCount(3)
        self.log_box.setHorizontalHeaderLabels(
            ['Timestamp', 'Source', 'Message'])
        header = self.log_box.horizontalHeader()
        header.resizeSection(0, 200)
        header.resizeSection(1, 100)
        header.setSectionResizeMode(2, QHeaderView.ResizeMode.Stretch)
        main_vertical_layout.addWidget(self.log_box)

        # Add the layout to a widget and set it as the central
        # widget of the window
        central_widget = QWidget()
        central_widget.setLayout(main_vertical_layout)
        self.setCentralWidget(central_widget)

        # Set the window title and show the window
        self.setWindowTitle('DDS Recorder controller')
        self.resize(700, 400)
        self.show()

    def dds_domain_dialog(self):
        """Create a dialog to update the DDS Domain."""
        dialog = DdsDomainDialog(self.dds_domain)
        if dialog.exec():
            domain = dialog.get_dds_domain()
            if (self.dds_controller.is_valid_dds_domain(domain)):
                self.restart_controller(dds_domain=domain)

    def start_clicked(self):
        """Publish START command."""
        # Publish start command
        self.dds_controller.start()
        # Update status box
        self.status_box.setText('Recording')
        self.status_box.setStyleSheet('background-color: green')
        # Print log
        self.add_log_entry('Controller', 'START command sent')

    def pause_clicked(self):
        """Publish PAUSE command."""
        # Publish pause command
        self.dds_controller.pause()
        # Update status box
        self.status_box.setText('Paused')
        self.status_box.setStyleSheet('background-color: orange')
        # Print log
        self.add_log_entry('Controller', 'PAUSE command sent')

    def stop_clicked(self):
        """Publish STOP command."""
        # Publish stop command
        self.dds_controller.stop()
        # Update status box
        self.status_box.setText('Stopped')
        self.status_box.setStyleSheet('background-color: red')
        # Print log
        self.add_log_entry('Controller', 'EVENT command sent')

    def event_clicked(self):
        """Publish EVENT command."""
        # Publish event command
        self.dds_controller.event()
        # Print log
        self.add_log_entry('Controller', 'EVENT command sent')

    def close_clicked(self):
        """Publish CLOSE command."""
        # Publish close command
        self.dds_controller.close()
        # Update status box
        self.status_box.setText('Closed')
        self.status_box.setStyleSheet('background-color: gray')
        # Print log
        self.add_log_entry('Controller', 'CLOSE command sent')

    def add_log_entry(self, source, message):
        """Add log entry to the logging table."""
        # Get the current datetime and format it
        timestamp = QDateTime.currentDateTime().toString(
            'yyyy-MM-dd hh:mm:ss,z')

        # Insert a new row in the table with timestamp, source, and message
        row_count = self.log_box.rowCount()
        self.log_box.insertRow(row_count)
        self.log_box.setItem(row_count, 0, QTableWidgetItem(timestamp))
        self.log_box.setItem(row_count, 1, QTableWidgetItem(source))
        self.log_box.setItem(row_count, 2, QTableWidgetItem(message))

    def closeEvent(self, event):
        """Update close event to close the application properly."""
        # Call the parent closeEvent to close the application
        super().closeEvent(event)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    controller_gui = ControllerGUI()
    controller_gui.show()
    sys.exit(app.exec())
