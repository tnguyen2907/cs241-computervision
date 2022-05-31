all: main

main: main.cpp calculate_depth_map_sgm.cpp fund_mat.cpp get_key_pts.cpp match_key_pts.cpp output.cpp rectify_img.cpp
	g++ main.cpp calculate_depth_map_sgm.cpp fund_mat.cpp get_key_pts.cpp match_key_pts.cpp output.cpp rectify_img.cpp

clean:
	rm -rf main