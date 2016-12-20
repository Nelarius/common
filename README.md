# common lib

This repository contains my code that is shared across multiple projects.

## Build

The `common.lua` contains functions representing premake projects.

* `test` will generate the unit test project
* `common` generates a static lib project containing the basic functionality of this lib (allocators)
* `gl3w` generates a static lib project for gl3w (OpenGL function loader)

Pass the location of the common folder to each function, without a trailing slash.

## Some assumptions about the owning project

The projects in `common.lua` assume that the following three configurations are defined: `Debug`, `Release`, and `Test`.
