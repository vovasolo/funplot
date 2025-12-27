# funplot
An interactive math function plotter with optinal function parameters adjustable in real time. 

## Description
A function of one variable (x) is given in the form of a c-like expression. Four standard operations (+, - *, /) as well as unary minus and exponentiation (^) are supported with the usual precedence order. Precedence can be changed by parentheses; multiple nested parentheses are supported. A set of about 20 math functions from the standard C math library can be used. Besides variable x, the expression parser recognizes user-defined named parameters, that can be adjusted by sliders altering the plot in real time. 

The expression parser and evaluator is based on [VFormula](https://github.com/vovasolo/vformula) class. 
The user interface is built on top of [Dear ImGui](https://github.com/ocornut/imgui) and and [ImPlot](https://github.com/epezent/implot) libraries. The graphical initialization is partially based on the code taken from the ImGui example application for SDL2 + OpenGL3.

## Installation
Prerequisits: SDL2 library, including development files. On Debian/Ubuntu 
```
apt install libsdl2-dev
```
should install all the necessary files. Two other dependencies, Dear ImGui and ImPlot can be pulled from the respective GitHub repositories:

```
mkdir funplot
cd funplot
git clone https://github.com/ocornut/imgui.git
git clone https://github.com/epezent/implot.git
git clone https://github.com/vovasolo/funplot.git
make
```

## Usage
Type the expression to plot into the top text input widget and click the "Plot" button. If parsing was successful, the next line will show "OK" and plot will appear in the graph window on the right. In case of a  parsing error, the next line will show "Err." and a caret pointing to the approximate position at which the error occurred. Edit the expression and click "Plot" again to fix the errror. You may need to zoom the plot area out using the mouse wheel to see the actual plot. Use left click + drag to pan the plotting area and right click + drag to define a rectangle area (marked yellow) to zoom into. If you change the expression, click the "Plot" button again to update the plot.

In this version only "x" is accepted as a variable name. There also can be three types of constants in your expression:
* numbers; both decimal and scientific notations are supported
* predefined named constants (currently, just one: pi=3.1415926...)
* user-defined named constants (parameters)
To define a parameter, click "Add parameter" button, a pop-up window appears. Fill the entries, then click "Add", then click somewhere outside the pop-up window to close it. A new slider will appear in the "Parameters" section, it can be now used to interactively adjust the parameter. From this point, the parameter name can be used in the expression. The plot will reflect the parameter changes in real time. 
