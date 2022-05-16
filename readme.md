# A CLI Interface Audio Unit

## Progress:
1. Basic Audio Routing Layouts - **Finished**
  - Audio Input Interface - **Finished**

    `AudioReceiver`, allows multiple device to connect, and multi-track audio input (supports various track format).
  - Audio Output Interface - **Finished**

     `AudioGenerator`, allow multitrack output

2. Basic ASIO Interface - **Under Progress**
  - ASIO Output Interface - **Finished**
  - ASIO Input Inteface - **To be done**
  - ASIO Driver Select & Ultilities - **In Progress**
    TODOs:
      1. Error Process when "Fail to Connect" happens

3. Basic WDM Interface - **To be done**

4. Basic Routing Units - **Under Progress**
  - Fader - **Finished**

    Fader, including MUTE button
  - Panner - **To be done**
  - Mixer - **To be done**

5. Basic Signaling Units - **Under Progress**
  - Basic Wave Generator - **Under Progress**

    Sine generator has been finished to test the routing facilities.
  - Waveform Player - **Finished**

6. Basic DSP Units - **Under Progress**
  - Basic Imager - **Finished**
    
    Simple and Basic Imager without Level Balance.
  - Basic Parametric EQ - **To be done**
  - Basic Dynamics Effects - **To be done**

7. Plugin Structure - **To be done**
  - VST Plugin Wrapper

## ASIO Issues
Because ASIO forbids distributing sourcecode, excluded all the ASIO source codes in .gitignore. But for convinience, I modified the ASIO folder layout, details could be found in `CMakeLists.txt`.
