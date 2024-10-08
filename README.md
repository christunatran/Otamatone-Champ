# install brew
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

echo >> /Users/tunapee/.zprofile
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> /Users/tunapee/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"

# issues while installing python libraries

```
tunapee@Christinas-MacBook-Air otamatone champ % pip install pyaudio numpy aubio pyautogui
```

```
brew install portaudio
```

```
pip install librosa
```

```
pip install numpy pyautogui
```

### was able to install all of the above libraries except for `aubio`


To use this script:

Adjust the MIN_PITCH and MAX_PITCH variables to match your Otamatone's range.
You may need to adjust bottomMarginSize and topMarginSize to match your Trombone Champ window setup.
Run the script before starting Trombone Champ.
Play your Otamatone near your computer's microphone.


# Doing a calibration to get the MIN_PITCH and MAX_PITCH using a small script

Detected pitch range:
MIN_PITCH = 144.09 Hz
MAX_PITCH = 1224.39 Hz

## problems
n_fft (Number of FFT points):

FFT stands for Fast Fourier Transform, a mathematical technique used to convert time-domain signals (like audio) into frequency-domain representations.
n_fft determines the size of the FFT window. It affects the frequency resolution of the analysis.
A larger n_fft gives better frequency resolution but poorer time resolution, and vice versa.
In our case, we're setting it to match our CHUNK size to avoid the warning about mismatched sizes.

Input buffer:

In audio processing, a buffer is a temporary storage area for audio data.
The input buffer holds chunks of audio data read from the microphone before they're processed.
When the buffer fills faster than the data can be processed, an "input overflow" occurs.

Stream:

In the context of PyAudio, a stream is an object that represents a continuous flow of audio data.
It's like a pipeline between your microphone (input device) and your Python script.
We open a stream to start receiving audio data and close it when we're done.
Operations like stream.read() get audio data from this pipeline.

#### Here's a simplified explanation of how these components work together in your script:

We open an audio stream connected to your microphone.
We repeatedly read chunks of audio data from this stream into our input buffer.
We process each chunk using FFT (via librosa) to detect the pitch.
Based on the detected pitch, we move the mouse cursor.
We repeat this process until the script is stopped.


### 1:54pm 
- ran otamatone_champ.py
- i do see the mouse going up and down, however it's happening on the middle of the screen
- i'm actually not interested in the audio stuff going on under the hood; i just want it to work
- modified the example code's 'mouse' library code to work with librosa and pyautogui code
- 


# TODO
- [ ] find a better way to open trombone champ 
- [ ] find better way to kill code