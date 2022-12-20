# godot-steam-extension

Godot Steam integration using GDExtension

This is a Work In Progress.

## Building this plugin
Before you can build this plugin please setup the [scons buildsystem](https://docs.godotengine.org/en/stable/development/compiling/index.html).

Make sure that when you clone this repository you initialise the submodules recursively.

Compile `godot-cpp` as per usual:
```
cd godot-cpp
scons target=template_debug generate_bindings=yes
cd ..
```

After this compile the plugin:
```
scons target=template_debug
```

> It is important to build debug builds as the editor requires debug builds to run.

To update the submodle run

```
git submodule update --remote
```