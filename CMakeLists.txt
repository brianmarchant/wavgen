cmake_minimum_required(VERSION 3.5)

project(wavgen LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(wavgen wavgen.c help.c log.c opts.c riff.c wf_output.c wf_counter.c wf_noise.c wf_saw.c wf_silence.c wf_sine.c wf_square.c wf_steps.c wf_markers.c wf_burst.c)
target_link_libraries(wavgen)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(wavgen PUBLIC ${MATH_LIBRARY})
endif()
