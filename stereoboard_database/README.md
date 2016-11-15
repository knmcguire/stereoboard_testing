Stereocamera Database

This folder has database images for the stm32F4 based Stereo Camera, developed by the MAVlab of the Delft university of Technology. 

The folder structure goes as follows:

Database_Stereoboard_1/ 
 - forward_camera/ (database taken with the stereo camera looking forward)
	- Take*/  (This consists of 22 takes)
		- *.bmp (images for database)
		- postion.dat (postion and orientation of the camera, recorded with optitrack with a timestamp)
		- timing.dat (timestamp of images, to be synced with optitrack data)
Database_Stereoboard_2 
 -downward_camera/  (database taken with the stereo camera looking downward)
	- Take*/ (This consist of 4 takes)
 -forward_camera/
	- Take*/ (This consist of 8 takes)

Each stereoboard database will contain an own README.md file with some additional information about the tests

____________________________

data file formats:


'position.dat'

 - timestamp of logging position data
 - timestamp from Optitrack system
 - X location 	[m]
 - Y location 	[m]  (vertical!)
 - Z location 	[m]
 - roll angle 	[deg]
 - pitch angle 	[deg]
 - yaw angle 	[deg]  (wrt negative-Z-axis)


'timing.dat'

 - timestamp of logging image
 - constant number 1 
