# What is vortxGE

vortxGE is a library written in c++, using  openGL and Bullet physics.
Focused on an easy to use base for game development on Linux.

### Modules
* [Animation](Animation)
* [Asset Management](Assets)
* [Controls](Controls)
* [Events](Events)
* [Lighting](Lighting)
* [Math Classes](Math)
* [Bullet Physics](Physics)
* [Properties](Properties)
* [Scene Nodes](Scene)
* [Shaders](Shader)


### build the vortxGE library
Inside the base dir

``` bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j2
```

Now if you want to install it into your system, Run

``` bash
sudo make install
```

To build the API documentation

``` bash
make apidoc
```

