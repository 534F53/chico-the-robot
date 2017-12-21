# Chico: The Robot #
This software is desired to be installed on a robot so that it would imitate the behavior of a pet Chihuahua.  
The following libraries/modules were provided: FreeRTOS system kernel (not included in the repo, but necessary for the project to work), custom_timer.c, motion.c, and wireless_interface.c

Users can control Chico via the web interface if **control mode** is selected.
Chico is default to stop if no command is sent; on forward/backward commands, it will move forward/backward around 1 meter correspondingly; on spin commands, it will spin 90 degrees to the specified direction.

On the other hand, if user select the **attachment mode**, Chico will start to move automatically. It has 3 states in the attachment mode:

* Searching state: Chico will start with searching state, it spins left and right to search heat sources. If a heat sources is found nearby, it will enter attached state, otherwise it will enter panic state after 5 seconds (The time is set to be 5 seconds so that we can see the behavior more easily during testing and demonstration).

* Attached state: Chico will stop movements once it enters attached state. However, if heat source moves away from it, Chico will try to catch up by moving forward until it finds the heat source again. Since the heat source following is optional in the requirements, we found this behavior satisfactory for the Chico to follow a heat source that moves straight. If Chico fails to catch up with the heat source, it will go back to searching state.

* Panic state: in this state, Chico will do nothing but spinning. This state will last for 5 seconds (again, for the testing and demonstration purpose, this period is set to be low) and then go back to searching state.

## Project Setup ##
* Clone the repo to your PC/laptop under freeRTOS folder
* Set up your eclipse for C development
* In eclipse, build the project
* Download the build to the robot using AVRDude
* Open Tera Term to view debug messages if necessary
