cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(rp2040_pwm_ntsc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(rp2040_pwm_ntsc
	main.c
	rp2040_pwm_ntsc_textgraph.c
	text_graph_library.c
	fontdata.c
	raytrace.c
)

# Pull in basic dependencies
target_link_libraries(rp2040_pwm_ntsc pico_stdlib hardware_pwm hardware_dma)

# create map/bin/hex file etc.
pico_add_extra_outputs(rp2040_pwm_ntsc)
