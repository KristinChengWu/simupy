# Tutorial on S-Function

The S-Function may require more explanation than the Python server side to understand. 
This tutorial will provide a brief guide on functions required to create the S-Function. 
See [this paper](https://www.uiam.sk/pc09/data/papers/061.pdf) and [sfuntmpl_doc.c](https://www.mathworks.com/help/simulink/sfg/templates-for-c-s-functions.html) for more information. 

## mdlInitializeSizes, mdlInitializeSampleTimes, and mdlStart

These functions are called once at the beginning of the simulation. 

`mdlInitializeSizes` is a mandatory function called to inquire about the number of input and output ports, sizes of the ports, etc. The important function to call are:
* `ssSetNumSFcnParams` defines parameters (in this implementation, there are none). 
* `ssSetNumInputPorts` and `ssSetInputPortWidth` defines the number of input ports and the size of the input (in this implementation there is just one input port of size one, the clock)

`mdlInitializeSampleTimes` is a mandatory function called to to set the sample times of the S-function. The important functions to call are:
* `ssSetSampleTime` sets the sample time
* `ssSetOffsetTime` ensures you sample at discrete equal-length intervals

`mdlStart` is an optional function. Here is where we initialize the socket and read from any files (see implementation for how to do that). We only need to do these tasks once, so it is more efficient to do this step here. 
* `#define MDL_START` must be included to indicate `mdlStart` is used
* Once the Simulink model has been compiled, it may still be necessary to pass in parameters to the executable (i.e., port number). This implementation does that using file I/O.
However, it is important to note that for compatibility with multiple systems, absolute paths should not be used. Here, the current working directory is used. 

## mdlOutputs

This is a mandatory function called at each time step to calculate the block outputs. This is where we send/receive data to/from the Python server. The functions called are:
* `ssGetInputPortSignal` and `ssGetOutputPortRealSignal` to get pointers to the input and output data, respectively. 

## mdlTerminate

This is a mandatory function called once at the end of the simulation. This is where we close the socket. 

## MEX

Compule the C S-function into a MEX file. It may be necessary to define libraries for compilation. Here is the command I used in the MATLAB terminal:

```
mex -g -LC:\EngApps\MATLAB\R2022b\sys\lcc64\lcc64\lib64 -lws2_32.lib client.c
```

