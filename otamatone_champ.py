import pyaudio
import numpy as np
import librosa
import pyautogui
import time

# Audio settings
CHUNK = 1024
FORMAT = pyaudio.paFloat32
CHANNELS = 1
RATE = 44100

# Game settings (you may need to adjust these)
screenWidth, screenHeight = pyautogui.size()
bottomMarginSize = 135
topMarginSize = 140
gameHeightInputSize = screenHeight - bottomMarginSize - topMarginSize

# Pitch range settings (adjust these to match your Otamatone's range)
MIN_PITCH = 144  # Hz
MAX_PITCH = 1224  # Hz

def pitch_to_y_coordinate(pitch):
    if pitch < MIN_PITCH:
        return gameHeightInputSize + topMarginSize
    elif pitch > MAX_PITCH:
        return topMarginSize
    else:
        pitch_range = MAX_PITCH - MIN_PITCH
        game_range = gameHeightInputSize
        normalized_pitch = (pitch - MIN_PITCH) / pitch_range
        return int(gameHeightInputSize * (1 - normalized_pitch)) + topMarginSize

# Variable to track if the mouse button is currently pressed
mouse_pressed = False

# Initialize PyAudio
p = pyaudio.PyAudio()
stream = p.open(format=FORMAT,
                channels=CHANNELS,
                rate=RATE,
                input=True,
                frames_per_buffer=CHUNK)

try:
    while True:
        try:
            data = np.frombuffer(stream.read(CHUNK, exception_on_overflow=False), dtype=np.float32)
            pitches, magnitudes = librosa.piptrack(y=data, sr=RATE, n_fft=CHUNK)
            index = magnitudes[:, 0].argmax()
            pitch = pitches[index, 0]

            if pitch > 0:  # A pitch of 0 means no pitch detected
                y = pitch_to_y_coordinate(pitch)
                pyautogui.moveTo(screenWidth//2, y)
                if not mouse_pressed:
                    pyautogui.mouseDown()
                    mouse_pressed = True
            else:
                if mouse_pressed:
                    pyautogui.mouseUp()
                    mouse_pressed = False
            
            time.sleep(0.01)  # Small delay to prevent overwhelming the system
        except IOError as e:
            print(f"Warning: {e}")
            time.sleep(0.1)  # Wait a bit before trying again

except KeyboardInterrupt:
    print("Stopped")
finally:
    if mouse_pressed:
        pyautogui.mouseUp()
    stream.stop_stream()
    stream.close()
    p.terminate()