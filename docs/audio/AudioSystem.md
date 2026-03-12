# Audio System

The audio system wraps OpenAL for 3D positional audio. It provides audio context management and error checking.

---

## lsOpenAL Class

Manages the OpenAL audio context. Defined in `vortxGE/src/openal/lsOpenAL.h`.

### Initialization

```cpp
lsOpenAL();
bool Init();
void Cleanup();
```

### Error Checking

```cpp
// Check for OpenAL errors
bool CheckALError(const string& filename, int line) const;

// Check for OpenALC errors
bool CheckALCError(ALCdevice* device, const string& filename, int line) const;
```

### Access

```cpp
ALCdevice* getDevice() const;
ALCcontext* getContext() const;
```

---

## Usage

### Initialization

```cpp
vortxGE* engine = new vortxGE();
engine->Init();  // This also initializes lsOpenAL

lsOpenAL* audio = engine->getOpenAL();
if (audio)
{
    // Audio system ready
}
```

### Cleanup

```cpp
// Cleanup is handled by vortxGE::Cleanup()
engine->Cleanup();
delete engine;
```

---

## Audio Components

### lsListener

Audio listener (usually attached to camera). Defined in `vortxGE/src/scene/lsSpeaker.h`.

```cpp
class lsListener
{
public:
    lsListener();
    ~lsListener();

    // Position and orientation
    void Set_Position(float x, float y, float z);
    void Set_Orientation(float x, float y, float z);

    // Velocity (for Doppler effect)
    void Set_Velocity(float x, float y, float z);
};
```

### lsSpeaker

Audio source that can be positioned in 3D space.

```cpp
class lsSpeaker
{
public:
    lsSpeaker();
    ~lsSpeaker();

    // Load audio file
    bool Load(const string& filename);

    // Playback control
    void Play();
    void Pause();
    void Stop();
    void Loop(bool loop);

    // Position (3D audio)
    void Set_Position(float x, float y, float z);

    // Volume
    void Set_Gain(float gain);

    // Pitch
    void Set_Pitch(float pitch);
};
```

---

## 3D Audio Setup

```cpp
// Create speaker
lsSpeaker* sound = new lsSpeaker();
sound->Load("data/audio/sfx/explosion.wav");
sound->Set_Position(10.0f, 0.0f, 5.0f);
sound->Set_Gain(0.8f);
scene->Add_Node(sound);

// Play
sound->Play();

// Update listener position (usually in camera update)
lsListener* listener = scene->getListener();
listener->Set_Position(camera->position.x, camera->position.y, camera->position.z);
listener->Set_Orientation(forward.x, forward.y, forward.z);
```

---

## Error Handling Macros

The engine provides macros for error checking:

```cpp
// Check after OpenAL call
AL_CHECK(alSourcePlay(source));

// Check and return on error
AL_RETURN_ON_ERROR(false);
```

---

## Notes

- The audio system is automatically initialized by `vortxGE::Init()`
- Use `.wav` files for audio (most compatible)
- For 3D audio, ensure the listener position is updated with the camera
- Distance attenuation is handled by OpenAL based on source/listener positions
