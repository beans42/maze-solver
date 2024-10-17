# maze-solve
Cross-platform maze-solving app using 'Dear ImGui' (GLFW/GLEW) for gui.

Implements [dijkstra's shortest path](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm), [a* search](https://en.wikipedia.org/wiki/A*_search_algorithm), [breadth-first search](https://en.wikipedia.org/wiki/Breadth-first_search), and [depth-first search](https://en.wikipedia.org/wiki/Depth-first_search).

Web demo available [here](http://beans42.github.io/maze-solver/), you can use sample mazes from [here](https://github.com/beans42/maze-solver/tree/master/sample%20mazes).

## build dependencies

- [GLFW](https://www.glfw.org/)
- [GLEW](http://glew.sourceforge.net/)
- one of these dialog-ing utilities (for tinyfd, only needed for linux)
  1. zenity / matedialog / qarma (zenity for qt)
  2. kdialog
  3. Xdialog
  4. python2 tkinter

## build instructions

windows: visual studio project files included

linux: run build.sh

## credits

- [ocornut/imgui](https://github.com/ocornut/imgui)
- [nothings/stb (stb_image.h and stb_image_write.h)](https://github.com/nothings/stb)
- [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/)
