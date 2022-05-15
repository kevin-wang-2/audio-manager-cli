cmake_minimum_required(VERSION 3.5)

project(audio_manager_cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ASIO
# Folder Structure:
# asio - inc    - all header files "*.h"
#      - host   - source files in "host" section of ASIO SDK in original layout
#      - common - source files in "common" section of ASIO SDK in original layout
include_directories(../asio/inc)

set(ASIO_SOURCES ../asio/common/asio.cpp ../asio/common/combase.cpp ../asio/common/debugmessage.cpp ../asio/common/register.cpp)
set(ASIO_SOURCES ${ASIO_SOURCES} ../asio/host/asiodrivers.cpp ../asio/host/ASIOConvertSamples.cpp)
set(ASIO_SOURCES ${ASIO_SOURCES} ../asio/host/pc/asiolist.cpp)

set(ASIO_HEADERS ../asio/inc/asio.h ../asio/inc/asiodrivers.h ../asio/inc/asiodrvr.h
    ../asio/inc/asiosys.h ../asio/inc/combase.h ../asio/inc/ginclude.h ../asio/inc/iasiodrv.h ../asio/inc/wxdebug.h ../asio/inc/asiolist.h)

# Project Codes
include_directories(drivers/abstraction)
include_directories(drivers/asio)

set(AUDIO_DRIVER_SOURCES drivers/abstraction/audiodriver.h drivers/abstraction/audiodriver.cpp
    drivers/abstraction/audiochannel.h drivers/abstraction/audiochannel.cpp)
set(AUDIO_DRIVER_SOURCES ${AUDIO_DRIVER_SOURCES} drivers/asio/asiodriver.h drivers/asio/asiodriver.cpp)

include_directories(processor/abstraction)

set(AUDIO_PROCESSOR_SOURCES processor/abstraction/audiogenerator.h processor/abstraction/audiogenerator.cpp
    processor/abstraction/audioreciever.h processor/abstraction/audioreciever.cpp
    processor/abstraction/audiodevice.h processor/abstraction/audiodevice.cpp)

include_directories(processor/generator)

set(AUDIO_GENERATOR_SOURCES processor/generator/sine.h processor/generator/sine.cpp
    processor/generator/waveformplayer.h processor/generator/waveformplayer.cpp)
set(AUDIO_PROCESSOR_SOURCES ${AUDIO_PROCESSOR_SOURCES} ${AUDIO_GENERATOR_SOURCES})

include_directories(processor/routing)
set(AUDIO_ROUTING_SOURCES processor/routing/fader.h processor/routing/fader.cpp processor/routing/Panner.cpp processor/routing/Panner.h)
set(AUDIO_PROCESSOR_SOURCES ${AUDIO_PROCESSOR_SOURCES} ${AUDIO_ROUTING_SOURCES})

include_directories(processor/effects)
set(AUDIO_EFFECTS_SOURCES processor/effects/Imager.cpp processor/effects/Imager.h)
set(AUDIO_PROCESSOR_SOURCES ${AUDIO_PROCESSOR_SOURCES} ${AUDIO_EFFECTS_SOURCES})

include_directories(timing)
set(TIMING_SOURCES timing/sampletimecode.h timing/sampletimecode.cpp)

include_directories(meta)
set(META meta/audiometa.h meta/AudioError.h)

set(CLI_ENTRANCE ${ASIO_SOURCES} ${ASIO_HEADERS} ${AUDIO_DRIVER_SOURCES}
    ${AUDIO_PROCESSOR_SOURCES} ${TIMING_SOURCES} ${META} main.cpp)

add_executable(asio_cli ${CLI_ENTRANCE})

target_link_libraries(asio_cli winmm.lib)

# Test Target
# (WAV) PCM Decoder

add_executable(pcm_test tools/PCM/pcm.cpp)
