CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(wavgen LANGUAGES C)

SET(WAVGEN_SOURCES
    help.c
    log.c
    opts.c
    riff.c
    wavgen.c
    wf_burst.c
    wf_counter.c
    wf_markers.c
    wf_noise.c
    wf_output.c
    wf_saw.c
    wf_silence.c
    wf_sine.c
    wf_square.c
    wf_steps.c
    )

ADD_EXECUTABLE(wavgen ${WAVGEN_SOURCES})
TARGET_LINK_LIBRARIES(wavgen)

INSTALL(TARGETS wavgen RUNTIME DESTINATION bin)

FIND_LIBRARY(MATH_LIBRARY m)
if(MATH_LIBRARY)
    TARGET_LINK_LIBRARIES(wavgen PUBLIC ${MATH_LIBRARY})
endif()
