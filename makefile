CXX = g++
CXXFLAGS = -std=c++11 -Wall -I/usr/include/opencv4
OPENCV_LIBS = -lopencv_stitching -lopencv_alphamat -lopencv_aruco -lopencv_barcode -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_cvv -lopencv_dnn_objdetect -lopencv_dnn_superres -lopencv_dpm -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_intensity_transform -lopencv_line_descriptor -lopencv_mcc -lopencv_quality -lopencv_rapid -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_shape -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres -lopencv_optflow -lopencv_surface_matching -lopencv_tracking -lopencv_highgui -lopencv_datasets -lopencv_text -lopencv_plot -lopencv_ml -lopencv_videostab -lopencv_videoio -lopencv_viz -lopencv_wechat_qrcode -lopencv_ximgproc -lopencv_video -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_dnn -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_imgproc -lopencv_core
LDFLAGS = $(OPENCV_LIBS) -lboost_system -pthread

SENDER_SRC = src/rpi/spoof.cpp 
RECEIVER_SRC = src/pc/receiver.cpp src/pc/pc_main.cpp src/pc/overlay_generator.cpp 
SENDER_TARGET = sender
RECEIVER_TARGET = receiver

all: $(SENDER_TARGET) $(RECEIVER_TARGET)

$(SENDER_TARGET): $(SENDER_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(RECEIVER_TARGET): $(RECEIVER_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean run_sender run_receiver

clean:
	rm -f $(SENDER_TARGET) $(RECEIVER_TARGET)

run_sender: $(SENDER_TARGET)
	./$(SENDER_TARGET) data/cropped_normal_video data/cropped_thermal_video 8080 8081

run_receiver: $(RECEIVER_TARGET)
	./$(RECEIVER_TARGET) localhost 8080 localhost 8081

full:
	bash run_project.sh