# funplot
An interactive math function plotter with optinal function parameters adjustable in real time. 

## Description
A function of one variable (x) is given in the form of a c-like expression. Four standard operations (+, - *, /) as well as unary minus and exponentiation (^) are supported with the usual precedence order. Precedence can be changed by parentheses; multiple nested parentheses are supported. A set of about 20 math functions from the standard C math library can be used. Besides variable x, the expression parser recognizes user-defined named parameters, that can be adjusted by sliders altering the plot in real time. 

The expression parser and evaluator is based on [VFormula](https://github.com/vovasolo/vformula) class. 
The user interface is built on top of Dear ImGui and and ImPlot libraries. The graphical initialization is partially based on the code taken from a ImGui example application for SDL2 + OpenGL.


