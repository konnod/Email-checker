# Email-checker
Electronic device based on NodeMCU and Arduino Uno used to check new incoming emails

Here i share the results of course project - source code of the email notification device based on Arduino UNO and ESP8266

The task was to design and create a stand-alone device that will periodically connect to user-defined email servers, and by a usage of the IMAP protocol will execute a request for user's mail.
The resulting device is able to connect to the Internet and work with the TCP / IP protocol stack.

Device is designed to inform the user about new messages when data is received from the server. The device gives visual representation of the data received in a human-readable form. Therefore, one of the constituent parts of the device is a LCD  display. 

There is also a sound alarm. For this purpose i use the sound generator - Buzzer. The frequency of the piezoelectric sound can be adjusted by programming the corresponding parameters. To prevent the device from making sounds at night, i use a photoresistor. By measuring the voltage level at the corresponding analog input, the microcontroller will decide whether to perform audible alarms.

User must enter connection data used for data retrieving from server. User needs to use the simple web form to input the imap server ip, port, email address and his password. The divece deploys the web server available on it's IP address received after connection to WiFi access point. 

The SSID and password of Access Point is set in the source code. It is possible to make the ESP module to deploy a WiFi AP and make the user to set the SSID and password of AP with Internet connection. Then the native AP must be shut down and module can do its job. But this is not implemented/

User's authorization data  is stored in the non-volatile memory of the device so that you do not have to enter data every time the device is restarted.

Connection to IMAP server is SSL secured, so it is impossible for attacker to get your credentials.

The ESP8266 has enough power to handle SSL, has a large memory size, has the ability to connect to a wireless network, supports the TCP / IP protocol stack and has a lightweight file system inside.

To control the signaling devices and visual alerts, the Arduino Uno card is used, which has a sufficient number of pins to connect these devices to it. Also, its use will reduce the load on the ESP8266.

The notification and signaling devices are LCD 16x2 display, LED 8x8 matrix and buzzer.

Device schematic:

