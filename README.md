# RealTimeGI
Real-Time Global Illumination - University of Leeds MSc Project.


# Build:
Visual Studio 2019
Vulkan SDK 1.2.162.1

Open the solution and build, if the Vulkan SDK not found change the project settings.

# How to use:
The software load the Sponza scene by default. with default Irradiance Volumes.</br>
Click Build to recompte the lighting.</br>

To load different scenes drag & drop .gltf files into the window. (.rtgi json files are used to load irradiance volumes see the example in Sponza directory).</br>
To edit/insert irradaince volumes create/edit .rtgi files with the same name as the .gltf file. reload the scene by drag & drop the new volumes will be created.</br>

Controls:</br>
1 key -> enable/disable global illumniation.</br>
2 key -> hide/show proxies.</br>

Navigation:</br>
you can move around the scene using w, a, s, d keys.</br>
holding right mouse click and move the mouse to look around.</br>


