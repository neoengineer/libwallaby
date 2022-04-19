#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "wpa_ctrl.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <wallaby/camera.h>
#include <wallaby/util.h>
#include <wallaby/tello.h>

#define BUFSIZE 1024
#define TELLO_LENGTH 13

#define TELLO_CMD_PORT 8889
#define TELLO_STATE_PORT 8890
#define TELLO_VIDEO_PORT 11111

static int tello_cmd_socket;
static struct sockaddr_in tello_cmd_addr;

int main(void)
{
	// int result;
	char buf[BUFSIZE];
	int len;
	int n;
	char * tellos;
	
	char cmd_buffer[128];
	int send_result;

	if( wpa_sup_connect() == -1)
	{
		printf("WPA Supplicant not active\n");
		return -1;
	}
	tellos = tellos_find();

	if(tellos == NULL)
		return;
	else
		printf ("Tellos: %s\n", tellos);

	// connect to the first tello on the list
	tello_connect(tellos);

	//wpa_cmd ("LIST_NETWORKS", buf);

	// Place the tello in sdk mode. We caanot porgress ustil this is done
	// so block until complete.
	do 
	{	
		send_result = tello_send("command");
		printf("send_result %d\n", send_result);
	
	} while(send_result != 0);

	// Turn  on video streaming from the tello
	tello_send("streamon");

	camera_open_device_model_at_res(0, TELLO, TELLO_RES);

	// Load a color channel configuration file
	printf("load config\n");fflush(NULL);
    int ret = camera_load_config("red");
    if (ret == 1) { 
		printf("...success\n");fflush(NULL);
	}

	printf("waiting for camera_update\n");fflush(NULL);
    camera_update();
    printf("Cam width x height:  %d x %d\n", get_camera_width(), get_camera_height());fflush(NULL);

    printf("Num channels = %d\n", get_channel_count());fflush(NULL);


	// Image processing has started so now launch the tello and start blob tracking


	// Launch the tello. This cammond will not return until the tello has completed the takeoff and
	// is holding position at the defeulat altitude


	// Autonomous loop...
	int i;
	double start = seconds();
	const int num_imgs = 600;
	for (i = 0; i < num_imgs; ++i)
	{
		// Process a video frame
		camera_update();

		// If an object is detected, the goal is to keep the tello centered on and at a constant
		// distance from the object. We can do this by tracking the largest blob and moving the
		// tello to keep the blob's center in the center of the display

		// The tello coordinate system is a top view looking down orientation
		// x values move the tello left, right
		// y values move the tello forward and backwards
		// z values increase and decrease the tello's altitude.

		// Blobs are tracked by size, with blob 0 being the largest blob found

		// The tello camera resolution is 1280 x 720, so the center of the image is 640, 360

		// Check if an object (blob) is dected one the zero color channel
		int obj_count = get_object_count(0);

		if (obj_count > 0)
		{
			// May want to add confidence and size checks
		
			// Retrieve blob zero's center coordinates
			int obj_center_x = get_object_center_x(0,0);
			int obj_center_y = get_object_center_y(0,0);
			
			printf("objs: %d (%d,%d)\n", obj_count, obj_center_x, obj_center_y); fflush(NULL);
	
			// Compute the blob's offset from center
			int obj_offset_x = obj_center_x - 640;
			int obj_offset_y = obj_center_y - 320;
			
			printf("offset (%d,%d)\n", obj_offset_x, obj_offset_y); fflush(NULL);

			// Compute x,z movement to keep the blob in the center of the image
			// If the blob is not in the center, we will make very small position adjustments
			// to prevent overshooting the target position
			int x_movement = 0;
			int y_movement = 0;
			int z_movement = 0;
					
			if (obj_offset_x < -64)			// obj is left of center, so move left
				x_movement = -10;
			else if (obj_offset_x > 64)		// obj is right of center, so move right
				x_movement = 10;
				
			if (obj_offset_y < -32)			// obj is below center, so descend
				z_movement = -10;
			else if (obj_offset_y > 32)		// obj is above center so ascend
				z_movement = 10;

			// Compute y movement to keep constant distance (fixed bounding box area)
			// Determine the area as a percentage of the total image size (128x720)
			float area = 921600 / get_object_area(0,0);
			printf("area = %f\n", area); fflush(NULL);

			if (area < 0.04)				// Area is small, so move forward
				y_movement = 10;
			else if (area > 0.4)			// Area is large, so move back
				y_movement = -10;
			// else: y_movement = 0
			
			printf("movement x=%d, y=%d, z=%d\n", x_movement, y_movement, z_movement); fflush(NULL);

			sprintf(cmd_buffer, "%d %d %d %d", x_movement, y_movement, z_movement, 0 );
			send_result = tello_send(cmd_buffer);
			printf("send_result %d\n", send_result); fflush(NULL);
			msleep(500);
		}
			
		else
		{
			// We do not see or lost track of the blob, so stop moving
			sprintf(cmd_buffer, "%d %d %d %d", 0, 0, 0, 0 );
			send_result = tello_send(cmd_buffer);
			printf("send_result %d\n", send_result); fflush(NULL);
			msleep(500);
		}                    
	}
		
	double stop = seconds();
	printf("%f fps\n", ((double)num_imgs/(stop-start)));

    printf("Camera close\n");
    camera_close();
    close (tello_cmd_socket);
    return 0;
}

