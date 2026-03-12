# Utilities

The utilities provide essential helper functions and classes for timing, file I/O, and string handling.

---

## Timer System

### lsTimer Class

High-resolution timer for delta time calculation. Defined in `vortxGE/src/timers/lsTimer.h`.

```cpp
class lsTimer
{
public:
    lsTimer();
    ~lsTimer();

    // Start timing
    void start_Timer();
    void start_Timer(double remaining, bool adjust);

    // Get elapsed time in milliseconds
    double get_ElapsedTime();

    // Sleep for milliseconds
    void sleep(double msec);
};
```

### Usage

```cpp
lsTimer timer;
timer.start_Timer();

while (running)
{
    // Get delta time in milliseconds
    double delta = timer.get_ElapsedTime();
    timer.start_Timer();

    // Update game
    update(delta);

    // Render
    render();

    // Cap frame rate
    if (delta < 16.67)  // ~60 FPS
    {
        timer.sleep(16.67 - delta);
    }
}
```

---

## File I/O

### lsFileIO Class

File reading and writing. Defined in `vortxGE/src/filesystem/lsFileIO.h`.

```cpp
class lsFileIO
{
public:
    lsFileIO();
    ~lsFileIO();

    // File operations
    bool File_Exists(const string& filename);
    bool Load_File(const string& filename);
    bool Save_File(const string& filename, const string& data);
    bool Delete_File(const string& filename);

    // Data access
    char* getData() const;
    long getSize() const;
};
```

### Usage

```cpp
lsFileIO file;

// Check if file exists
if (file.File_Exists("data/level.txt"))
{
    // Load file
    if (file.Load_File("data/level.txt"))
    {
        // Process data
        const char* data = file.getData();
        long size = file.getSize();
    }
}
```

---

### lsDir Class

Directory operations. Defined in `vortxGE/src/filesystem/lsDir.h`.

```cpp
class lsDir
{
public:
    lsDir();
    ~lsDir();

    // Directory operations
    bool Dir_Exists(const string& path);
    bool Create_Dir(const string& path);
    bool Delete_Dir(const string& path);

    // List contents
    bool List_Dir(const string& path);
    string getFilename(int index) const;
    int getFilecount() const;
};
```

### Usage

```cpp
lsDir dir;

// List files in directory
if (dir.List_Dir("data/meshes"))
{
    for (int i = 0; i < dir.getFilecount(); i++)
    {
        cout << dir.getFilename(i) << endl;
    }
}
```

---

## String Utilities

### lsString Class

String manipulation. Defined in `vortxGE/src/datablocks/lsString.h`.

```cpp
class lsString
{
public:
    lsString();
    lsString(const string& str);
    ~lsString();

    // String operations
    void toUpper();
    void toLower();
    void trim();
    void replace(const string& from, const string& to);

    // Split into tokens
    vector<string> split(char delimiter) const;

    // Convert to/from numbers
    int toInt() const;
    float toFloat() const;
    double toDouble() const;

    static string fromInt(int value);
    static string fromFloat(float value);
};
```

### Usage

```cpp
lsString str("  Hello, World!  ");
str.trim();  // "Hello, World!"
str.toUpper();  // "HELLO, WORLD!"

// Split
vector<string> tokens = str.split(',');  // ["Hello", " World!"]

// Convert
int num = lsString("123").toInt();  // 123
string s = lsString::fromInt(456);  // "456"
```

---

## Debug Utilities

### Debug Flags

Defined in `vortxGE/src/vortx_debug.h`:

```cpp
// Enable debug output by uncommenting
// #define VORTX_DEBUG_STATS
// #define VORTX_DEBUG_RENDER
// #define VORTX_DEBUG_SHADERS
// #define VORTX_DEBUG_ASSETS
// #define VORTX_DEBUG_ANIMATION
// #define VORTX_DEBUG_PHYSICS
// #define VORTX_DEBUG_LOADSCENE
```

### Debug Output

```cpp
#include "vortx_debug.h"

#ifdef VORTX_DEBUG_RENDER
    cout << "Rendering frame " << frame << endl;
#endif
```
