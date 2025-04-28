single:
	python3 1.py input.mp4 single output_single_temp.mp4
	ffmpeg -i output_single_temp.mp4 -c:v libx264 -preset medium -crf 23 output_single.mp4 -y
	rm output_single_temp.mp4

multi8:
	python3 1.py input.mp4 multi output_multi_8_temp.mp4 --workers 8
	ffmpeg -i output_multi_8_temp.mp4 -c:v libx264 -preset medium -crf 23 output_multi_8.mp4 -y
	rm output_multi_8_temp.mp4

multi2:
	python3 1.py input.mp4 multi output_multi_2_temp.mp4 --workers 2
	ffmpeg -i output_multi_2_temp.mp4 -c:v libx264 -preset medium -crf 23 output_multi_2.mp4 -y
	rm output_multi_2_temp.mp4

multi4:
	python3 1.py input.mp4 multi output_multi_4_temp.mp4 --workers 4
	ffmpeg -i output_multi_4_temp.mp4 -c:v libx264 -preset medium -crf 23 output_multi_4.mp4 -y
	rm output_multi_4_temp.mp4

multi16:
	python3 1.py input.mp4 multi output_multi_16_temp.mp4 --workers 16
	ffmpeg -i output_multi_16_temp.mp4 -c:v libx264 -preset medium -crf 23 output_multi_16.mp4 -y
	rm output_multi_16_temp.mp4

all: single multi2 multi4 multi8
