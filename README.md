# esp32-bluetooth-tracker

A bluetooth tracking application for the esp32. Connects multiple esp32's together via a mesh network (PainlessMesh). All nodes will share the amount of nearby devices based on active BLE signals.

# Usage

Change the constant of the node id for each node. The node with the id the same as the 'master id' will write the data from all nodes within the network to the serial monitor.

Upload the code to various nodes, using for example the Arduino IDE, and plug the master node into your computer to read out all the data.

# Issues

- Sometimes not all nodes want to share their data or even connect to the network at all.
- The way the JSON objects are created is very unsecure when it comes to memory allocation.
- Some nodes can crash and restart. This happens frequently.

# Notes

I am an inexperienced developer, now going into my second year of a CS Bachelor's degree. This piece of code has a lot of issues so any help is very welcome!
