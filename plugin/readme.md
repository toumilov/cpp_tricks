# Plugin

This sample provides solution for application extension development.
Each extension is a shared library that conforms to interface.

# Properties
* **name** - extension (feature) name
* **name space** - extension category (group)
* **version** - extension verion: major, minor, patch

# Interface
Extension library provides dynamic APIs for loader:
* **plugin_id** - extension identity function
* **plugin_instance** - extension instance builder

# Instance
Plugin instance must inherit from **_plugin::Instance_** and should implement identity method **_id()_**.

# Loading
Extension loader provides API to search extensions in specified location.
It's up to client to decide which of found extensions to load and when to unload them.

# Example
Extenstion library: **_libplugin.so_**

Custom extension interface: **_my_plugin.hpp_**

Sample extension: **_test_plugin.cpp_**

Extension user application: **_app.cpp_**
