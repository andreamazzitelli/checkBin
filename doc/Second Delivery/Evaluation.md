## Energy Consumption
A major constraint that we have is energy consumption. We want the battery to last for at least a year. During the next development phase we’ll choose if we need to add some kind of solar cell or other type of charging methods. As an example we could experiment with some kind of fast charge while the trucks unload the bins.
- The OLED screen will be always on and will be updated at every change of the fill level.
- The stepper motor will be activated only when the bin is full or recently emptied.
- The sensors will be activated at the predefined intervals. The ultrasonic sensor will take three measures with a 30 seconds interval, to discard eventual measures taken while the bin was open. The load cell will also be activated to read a value.

### Analysis on sampling frequency:
One of the key aspects of the project is how often the sampling is done. Indeed, there are several possibilities:
- Continuous sampling of all the sensors: this first approach is clearly the simplest one. It allows high accuracy and responsiveness to changes but at the cost of always consuming energy. In particular, we have that keeping ON the load cell would cost 1.5mA ([datasheet](https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf)) and keeping ON the ultrasonic would cost 15mA ([datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)).
<br>This totals up to 16.5 mA. Considering a 1000mAh battery only for the sensors, they would run for 60.61 hours, which is almost 3 days. That result is clearly unacceptable for an application like ours. Indeed, the goal is to have a battery which lasts for no less than a year, also considering that it must give power also to the board and to the actuators.
<br>Moreover, continuous sensing does not make much sense in our use case. Seeing the nature of what we are measuring (i.e. people throwing away garbage), we expect random changes in the values with random intervals between them (also long periods of time in which the values do not change). This behavior clearly describes discrete events and not continuous ones, strengthening our claim of continuous sensing being unnecessary.
- Continuous sensing of one sensor and sensing of the other only when needed: this approach may be a viable solution that allows us to promptly detect changes in the environment, using the other sensor only as confirmation for the action to take. This method falls in the same shortcomings of the previous one, especially regarding the energy consumption. In particular, if we keep the load cell always ON with a battery of 1000 mAh, it would last for 27.75 days. This result is clearly an improvement but still not nearly enough for our goal of changing the battery not more than once a year.
<br>If keeping the load cell always on is not a good solution, clearly keeping on the ultrasonic sensor (that uses 10 times more mA than the load cell) is not a viable possibility. Moreover the considerations done on the nature of the environment still holds.
- Periodic sensing: this is the solution that we went for. We believe it is a good compromise between energy consumption and data availability. It is also possible to tailor the interval between measurements, during the development we choose for our application an interval of 1 hour.
<br>We came up with this value using some empirical observations and some reasoning: considering that in Rome the bins get emptied on average every 2 days and that our “fill level scale” is 0-9, we deduced that to accurately detect every level we would need to measure at least every 48/10 = 4.8 hours. Using a bigger interval we would lose accuracy in detecting when the fill level changes. Moreover we need to have a recent enough update so that the trucks can have a realist view to plan their route.
<br>A possible solution to allow trucks for better planning is to allow for sensing on request. This would imply keeping the radio always on which is unfeasible due to battery constraints. <br>In particular we chose a one hour interval because it allows us to cope with times in which there is more frequent activity (people tend to throw the garbage in defined times of the day, in the morning and in the evening). Sensing at intervals less than an hour would increase energy consumption and in normal situations we do not expect big changes in the fill level during this period of time.
<br>Assuming to use this method, we would keep both sensors ON for a few seconds every hour, which means roughly we would consume 16.5mA that corresponds to a battery lifetime of almost 5 years (assuming a 1000 mAh battery only for the sensors, one sensing every hour and 5 seconds to startup, sense and shutdown).
- We could introduce some machine learning techniques to tailor the sampling rate to the real needs. Having collected enough data on the times of the day when the bins are more used, we could know when sensing was useless (because the fill level was unchanged) and when it should have been more frequent. As a consequence we could adapt the sampling rate depending on this. We could also do a step forward by changing the sampling rate of each single bin, based on its specific needs.

### Analysis on radio usage: 
The second aspect that has a great impact on the consumption of energy is how often the radio is used. We came up with several possibilities:
- Transmit every new value: we would transmit data every time the sensors perform a new measurement. This solution allows to keep the radio active only a few seconds every timeout. But we do not need to know if a fill level has not changed, as for our system we are only interested in fill level changes.
- Transmit every time a fill level changes: in this case we would transmit data every time the fill level of the bin changes with regard to the previous measurement. We reduced even more the energy consumption with respect to the previous case, without losing any relevant information on the status of the bins.

The antenna will be also activated when an anomaly is detected between the fill level measure taken by the ultrasonic and the weight measured by the load cell. In that case it will be used to notify the anomaly, sending the max fill level to the cloud to notify the bin must be emptied.

In conclusion we choose to sense every hour and to send data every time there is a change in the fill level of the bin or there is an anomaly.

## Sensors precision
The precision of the fill level depends on the height of the bin. The fill level is a value between 0 and 9, computed as `fill_level = 9-(10*distance/max_distance)`. Consequently every step of the fill level will represent a height value of the trash between `fill_level*max_distance/10` and `(fill_level+1)*max_distance/10`. As a consequence we have an intrinsic error percentage of maximum 10% of the total height, which is tolerable for our application.

We measured the precision of the load cell with respect to the conversion formula we used. From the tests we have done we discovered a 2% error, which is acceptable. It is also counterbalanced by the 20% margin used when comparing weights in the code.

We conducted some tests to evaluate the accuracy of the system. 
<br> Total measurements: 40.

| Fill level from ultrasonic sensor | Total measurements | Detected anomalies | Undetected anomalies |
| ----------- | ----------- | ----------- | ----------- |
| 0 | 2 | 0 | 0 |
| 1 | 3 | 0 | 0 |
| 2 | 5 | 0 | 0 |
| 3 | 4 | 0 | 0 |
| 4 | 7 | 0 | 0 |
| 5 | 6 | 1 | 0 |
| 6 | 3 | 0 | 0 |
| 7 | 3 | 1 | 0 |
| 8 | 4 | 0 | 1 |
| 9 | 3 | 0 | 0 |
| ----------- | ----------- | ----------- | ----------- |
| Total | 40 | 2 | 1 |

Accuracy of the fill level: 92.5%. We expect the accuracy to rise with a greater sample dataset.


## Network usage

This architecture does not have any particular network constraint. As we planned to send data to the cloud only when the fill level changes or if there is an anomaly, the board should send around 10 messages every interval between two bin unloadings. This choice has been made considering the nature of the system which does not need real time updates.

The great number of bins dislocated throughout the city should be supported by the gateway infrastructure.

The latency measured using the prototype from the sensing to the update of the web dashboard is less than 2 seconds, which is more than enough for our use case.

We’ll send to the cloud only the fill level of the bins and the bin identifier, so the bandwidth needed by each device is minimal. The size of the payload sent using LoRa to the cloud is less than 10bytes (1 for the fill level, 1 for the separator character and up to 8 for the bin identifier). To reduce the network usage we could also remove the bin identifier from the message, using the DEV_EUI assigned to the board as bin identifier.

We plan to test the whole system using a simulated environment provided by IoT_LAB. In this way we will also analyze the scalability of our system.

### Link to previous version: [Evaluation - First delivery](../First%20Delivery/Evaluation.md)
