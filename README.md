# IoT-Production-Device-Monitoring
A c based code applied to two microcontrollers to operate as a sensor data processing unit and operator to send the data to the cloud.

## Two Microcontrollers was used to monitor two production devices (A and B). ##
The production device A is a an oven that is used to dry foods. It is the device has three compartments, each with its heaters and fans. The device itself is equipped with a thermostat, but it does not show the actual temperature of each compartments. Therefore, a microcontroller (ESP32) connected to 3 LM 35 temperature sensors and WiFi module is used to monitor and send the temperatures to the cloud.

The production device B is a spinning drum, blown by a heated blower that is also used to dry foods. It only has one compartment but the drum is the compartment itself. The device itself is also equipped with a thermostat to control the blower temperature and show the temperature of the blower tube, but the temperature data can only be seen directly from the thermostat. Therefore, a microcontroller (ESP8266) connected to an infrared sensor to count the rotation of the drum, an LM35 temperature sensor, and WiFi module is used to monitor and send the temperatures and RPM to the cloud.
