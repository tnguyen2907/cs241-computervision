Trung Nguyen & Brian Do & Nam Anh Nguyen & Dat Le
05/30/2022
Stereo Reconstruction Project - CS 241

To run, in first part of main.cpp
	- Set calib_path to the path of calibration file. 
      Calibration file has to have at least 2 lines:
      ________
      vmin=0;
      vmax=100;
      ________
      vmin is the min disparity (default can be 0)
      vmax is the max disparity (default can be 100)
      This is for visualization of the depth map. You can change the value of vmax to find the best visualization
      Our data already has calibration file calib.txt

    - Set left_img_path to the path of the left image

    - Set right_img_path to the path of the right image

    - Set depth_map_path to the path of a text file that you want to save the depth map



We try to rectify our images, but it didn't work as we expected so we comment out the part that rectify images. That part has 3 section
    - First part is Finding key points and match key points
    - Second part is Find fundamental matrix
    - Third part is Rotate the image
Uncomment each part to see the result of that part