![libtwo](media/libtwo0s.png?raw=true)

| CI            | Community     | Support   |
| ------------- | ------------- | --------- |
[![Build Status](https://travis-ci.org/hugoam/two.svg?branch=master)](https://travis-ci.org/hugoam/two) [![Build status](https://ci.appveyor.com/api/projects/status/4a77n3scl71ybcgj?svg=true)](https://ci.appveyor.com/project/hugoam/two) | [![Gitter](https://badges.gitter.im/hugoam/two.svg)](https://gitter.im/hugoam/two) | [![Patreon](https://img.shields.io/badge/donate-patreon-orange.svg)](https://www.patreon.com/bePatron?u=11301355) [![Join the chat at https://gitter.im/hugoam/two](https://badges.gitter.im/hugoam/two.svg)](https://gitter.im/hugoam/two?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

two is an all-purpose c++ app prototyping library, focused towards live graphical apps and games.  
two contains all the essential building blocks to develop lean c++ apps from scratch, providing [reflection](#reflection) and low level [generic](#generic-features) algorithms, an [immediate ui](#ui) paradigm, and an immediate minimalistic and flexible [graphics renderer](#graphics).

two aims to provide the *quickest idioms* to build functional and lightweight c++ graphical applications. It wagers that the future of application and game coding lies in small, self-contained, reusable and shared libraries, and **not** in gigantic tightly coupled *engines* of hundreds thousands of lines of code. It tackles the problem of the code you *don't* want to write, and *should not have* to write when prototyping an app.

two provides two main APIs, two low-level application building blocks which you can use just as well from native C++ as from higher-level language bindings:

- a stable API for [mid-level graphics rendering](https://github.com/hugoam/two/blob/master/docs/graphics.md) (think scenes, shapes, meshes, models, lights, materials, passes, post-processing, etc).
- a stable API for fully auto-layout, stylable, [declarative/immediate mode UI](https://github.com/hugoam/two/blob/master/docs/ui.md) (with a large set of widgets, docking, tabs, nodes, text editors, inputs, curves etc).

Each of those layers are also their own libraries, available as [twfx](https://github.com/hugoam/twfx) and [twui](https://github.com/hugoam/twui), which can be used independently: they are not tied to one another.

**two** is open-source, and published under the zlib license: as such it is looking for sponsors, funding, and your support through [patreon](https://www.patreon.com/libmud).

### live coding

As a demonstration of the features and API of two, you can try this [live-coding environment](https://hugoam.github.io/two/editor/editor.html), which allows you to browse and edit (in real-time) any of the 35 examples we ported from three.js

[![samples](media/samples0.gif?raw=true)](https://hugoam.github.io/two/editor/editor.html)

### building
two is built with [GENie](https://github.com/bkaradzic/GENie) build system, which is based on premake and consists of a bunch of lua scripts. The GENie binaries needed by two for windows and linux are included for convenience in the `bin` folder.  
To get a headstart and build two you should clone this repository, and run GENie which will generate projects for two libraries and for the examples.

The build instructions for linux and gcc look like:
- `git clone https://github.com/bkaradzic/bx`
- `git clone https://github.com/hugoam/bimg`
- `git clone https://github.com/hugoam/bgfx`
- `git clone --recursive https://github.com/hugoam/two`
- `cd two`
- for linux (make):
  - `bin/linux/genie --gcc=linux-gcc gmake`
  - `cd build/projects/gmake-linux`
  - `make config=debug64 -j8`
- for osx (make): `bin/darwin/genie --gcc=osx gmake`
- for osx (xcode): `bin/darwin/genie --xcode=osx xcode9`
- for windows (visual studio): `bin/windows/genie vs2017`

### quickstart
This is the minimal sample code you need to run a two application
```c++
#include <two/two.h>

using namespace two;

bool pump(Shell& app)
{
    // ui code goes here
    return app.pump();
}

int main(int argc, char *argv[])
{
    Shell app(RESOURCE_PATH);
    app.run(pump);
}
```

Add some UI code or some graphics code in the body of the function to get started using two.  
Don't hesitate to have a look at the examples!

![samples](media/ui0.gif?raw=true)

### [ui](docs/ui.md)

two ui uses a novel paradigm that sits halfway between **immediate** (like dear imgui) and **retained** ui (like Qt) : its API looks and feels exactly like an immediate ui library, except not much is *actually* done immediately. As such, we prefer to refer to it as a **declarative** ui.  
The final tree of widgets will look exactly like the tree of the declarations that is traversed on any given frame. however, events are processed, and rendering is done in a separate step.

Once you have setup a window and called the `begin()` function on the root widget on each iteration, you can freely declare/draw all your widgets:
```c++
Widget& window = ui::window(uroot, "My Window");
ui::label(window, "Welcome to two ui");
if(ui::button(window, "Click me !"))
    printf("Button clicked !\n");
ui::color_edit(window, Colour(1.f, 0.34f, 0.73f));
```

The styles of all widgets are entirely customizable, through style sheets declared in the json format  
styles govern literally all aspects of :
- the layout of the widgets
- the appearance of the widgets

By switching between [style sheets]() on the fly, you can instantly change the **whole** appearance of the ui

You can also specify styles on a per-widget basis, by passing in a style parameter
```c++
Style style = {};
ui::button(parent, style, "Click me!");
```

An extensive UI example demonstrates the usage in a way no documentation ever could: have a look at the code.

### [graphics](graphics.md)
two gfx library uses the same immediate paradigm as the ui. instead of nesting ui nodes (widgets) calls, you nest graphics nodes calls. as such it is perfect for quickly setting up some debug graphics rendering.

```c++
// create a viewer to render into
SceneViewer& viewer = ui::scene_viewer(uroot);
Gnode& groot = viewer.m_scene.m_graph.begin();

// draw a node, which transform applies to children of this node
Gnode& gnode gfx::node(root, {}, vec3(0.f, 15.f, 7.5f));

// draw a cube shape item as a child of node
gfx::shape(gnode, Symbol(Colour::White), Cube());

// draw a 3d model item as a child of node
Item& item = gfx::model(gnode, "my_3d_model.obj");

// animate a model
Animated& animated = gfx::animated(gnode, item);
animated.play("walk");
```

A collection of 35 examples ported from three.js demonstrate most usages of the API: check them out to get familiar with how to use two.

### examples

#### [pbr materials](https://hugoam.github.io/two-io/examples/03_materials_low.html)
![pbr materials](https://github.com/hugoam/two-io/blob/master/media/03_materials.png)

#### [lights](https://hugoam.github.io/two-io/examples/04_lights.html)
![lights](https://github.com/hugoam/two-io/blob/master/media/04_lights.png)

#### [sponza (.obj import)](https://hugoam.github.io/two-io/examples/04_sponza.html)
![sponza (.obj import)](https://github.com/hugoam/two-io/blob/master/media/04_sponza.png)

#### [character (animations)](https://hugoam.github.io/two-io/examples/05_character.html)
![character (animations)](https://github.com/hugoam/two-io/blob/master/media/05_character.png)

#### [particles](https://hugoam.github.io/two-io/examples/06_particles.html)
![particles](https://github.com/hugoam/two-io/blob/master/media/06_particles.png)

#### [gltf](https://hugoam.github.io/two-io/examples/07_gltf.html)
![gltf](https://github.com/hugoam/two-io/blob/master/media/07_gltf.png)

#### [sky (perez model)](https://hugoam.github.io/two-io/examples/08_sky.html)
![sky (perez model)](https://github.com/hugoam/two-io/blob/master/media/08_sky.png)

#### [live shader](https://hugoam.github.io/two-io/examples/09_live_shader.html)
![live shader](https://github.com/hugoam/two-io/blob/master/media/09_live_shader.png)

#### [live graphics](https://hugoam.github.io/two-io/examples/14_live_gfx.html)
![live graphics](https://github.com/hugoam/two-io/blob/master/media/14_live_gfx.png)

#### [live graphics (visual script)](https://hugoam.github.io/two-io/examples/4_live_gfx_visual.html)
![live graphics (visual script)](https://github.com/hugoam/two-io/blob/master/media/14_live_gfx_visual.png)

### credits
two couldn't exist without:
- [GENie](https://github.com/bkaradzic/GENie) build system
- [bgfx](https://github.com/bkaradzic/bgfx) rendering library
- [vg-renderer](https://github.com/jdryg/vg-renderer) and [NanoVG](https://github.com/memononen/nanovg) vector drawing libraries
- [lua](https://github.com/lua/lua) scripting language
- [stb](https://github.com/nothings/stb) headers
- [glm](https://github.com/g-truc/glm) math library
- [json](https://github.com/nlohmann/json) header

### support
Creating two has been a huge time investment over the course of a few years: the only way I can pursue that effort and make it thrive into the programming ecosystem of our dreams, is through funding and sponsorship: you are welcome to have a look at our [patreon](https://www.patreon.com/toyengine).

Iron supports:
- Mike King

Stone supports:
- Etienne Balit, Le Bach, Manos Agelidis, Nebo Milic, Omar Cornut, Stefan Hagen, Sunder Iyer

### license
two is licensed under the [zlib license](LICENSE.txt).
