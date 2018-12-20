------- Assignment 2 (Roller Coaster) ---------
Class: CSCI 420
Author: Matthew Robinson
Id: 9801107911
Dev Environment: Mac OSX High Sierra
-----------------------------------------------

======================= Description ==============================
This uses Catmull-Rom splines along with OpenGL lighting and 
texture mapping to create a roller coaster simulation. The 
simulation will run in a first-person view, allowing the
user to "ride" the coaster in an immersive environment. 
==================================================================

====================== Compiling =================================
> unzip RollerCoaster.zip
> cd pic
> make
> cd ..
> cd assign2
> make
> ./RollerCoaster track.txt
===================================================================

====================== Keyboard Inputs ============================
(T or t Key) - Toggles the Default input from the Mouse
               to translate the Height Field.
               Upon the user pressing down on the left mouse button
               and moving the cursor, The Height Field will be translated
               in either the x,y direction depending on mouse movement.
               Upon the user pressing down on the middle mouse button
               and moving the cursor, The Height Field will be translated
               in the z direction.

(S or s Key) - Toggles the Screen Shot Capture Mode. During Capture
               Mode a Screen Shot will be taken once every 4 frames up 
	       to a total of 958 captures. Log Statements appear
               in the console to indicate a successful completion
	       of a Screen Capture. However, it should be noted
	       that Screen Capture Mode will slow down the processing
	       of the Roller Coaster Ride

(Z or z Key) - Moves the Camera in the -Y Direction

(X or x Key) - Moves the Camera in the +Y Direction

(Left Arrow Key) - Moves the Camera in the +Z Direction

(Right Arrow Key) - Moves the Camera in the -Z Direction

(Up Arrow Key) - Moves the Camera in the -X Direction

(Down Arrow Key) - Moves the Camera in the +X Direction

===================================================================

========================= Mouse Inputs ============================
Rotation - This is the Default Case (T has not been Toggled).
           Upon the user pressing down on the left mouse button
           and moving the cursor, The Height Field will be rotated in
           either the x,y direction depending on mouse movement.
           Upon the user pressing down on the middle mouse button and
           moving the cursor, the Height Field will be rotated
           in the z direction.

Scaling - To Scale the image, the Shift Key also has to be depressed
          before any mouse operation. Then upon the user pressing down
          on the left mouse button and moving the cursor, The Height Field
          will be scaled in either the x,y direction depending on mouse movement.
          Upon the user pressing down on the middle mouse button and
          moving the cursor, the Height Field will be scaled
          in the z direction.

Translation - To Translate the image, the T/t key has to have been 
              toggled on first. Then upon the user pressing down
              on the left mouse button and moving the cursor, The Height Field
              will be translate in either the x,y direction depending on mouse movement.
              Upon the user pressing down on the middle mouse button and
              moving the cursor, the Height Field will be translated
              in the z direction.
===================================================================

====================== Acknowledgments ============================
The Skybox Texture Images are credited to

	Jockum Skoglund aka hipshot
	hipshot@zfight.com
	www.zfight.com
	Stockholm, 2005 08 25

The Ground Image of the Mountains is credited to

	From the Icy Mountains | Cities XL Wiki
     	(user: Nicko2u)


===================================================================

========================= Extra Credit ============================
For Extra Credit I Implemented the Following
	- Rendered a Triple Rail (Common for Steel Roller Coasters)
	- Rendered two T-Shaped Rail Cross-Sections connecting
          the left and right rail to the center rail
        - The Track is Circular. (Albeit slightly unconnected due
                                  to curvature of the track)
	- Rendered a Support Structure for the Track
	- Modified the Velocity of which the camera moves to 
	  make it physically realistic in terms of gravity
          (Used Equation Provided)
	- Points for Challenging Roller Coaster Loop?
===================================================================

============================= Notes ===============================
RollerCoaster.mov - Animation Rendered at 24 frames per sec
===================================================================