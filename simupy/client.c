/* 
@author Kristin Cheng Wu
@date 2023-02-16
@desciption This S-Function acts as a client and communicates with a Python server through TCP/IP. 

Based off of sim_SendToXPlane.c, sim_RecvFromXPlane.c and https://www.uiam.sk/pc09/data/papers/061.pdf
*/

#define S_FUNCTION_NAME client
#define S_FUNCTION_LEVEL 2

// imports 
#include "simstruc.h" 
#include <winsock2.h>				
#pragma comment(lib, "ws2_32.lib") 	
#include <unistd.h>
#include <stdio.h>

// constants
#define N_INPUT 1 
#define N_OUTPUT 1
#define SEND_SIZE N_INPUT*8
#define RECV_SIZE N_OUTPUT*8

#define N_PARAMS 0
#define LOCALHOST "127.0.0.1"

// global variables 
SOCKET			Socket;
SOCKADDR_IN		Addr;

/* SetupSocket - Returns 1 upon success. 
 * @port: The port number to connect to. 
 * Description: Setup socket for TCP/IP communication.  
 */
int SetupSocket(SimStruct *S, int port)
{
	WSADATA wsa;
	int iOptVal = 0;
    unsigned int recvSize = RECV_SIZE;
    unsigned long int NonBlock = 0;	// Use blocking socket
    char error_message[80];

	// initialize winsock
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
        sprintf(error_message, "Failed, error code: %d", WSAGetLastError());
        ssPrintf(error_message);
		ssSetErrorStatus(S, error_message);
		return 0;
	}

	// setup socket
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == SOCKET_ERROR)
	{
        sprintf(error_message, "Failed to create send socket: %d", WSAGetLastError());
        ssPrintf(error_message);
		ssSetErrorStatus(S, error_message);
		return 0;
	}

	ioctlsocket(Socket, FIONBIO, &NonBlock);
	setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&iOptVal, sizeof(int));
    setsockopt(Socket, SOL_SOCKET, SO_RCVBUF, (char*)&recvSize, sizeof(unsigned int));

    //struct timeval timeout;
    //timeout.tv_sec = 10;
    //timeout.tv_usec = 0;
    //setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    //setsockopt(Socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

	// setup address structure
	Addr.sin_family			= AF_INET;
	Addr.sin_port			= htons(port);
	Addr.sin_addr.s_addr	= inet_addr(LOCALHOST);
    ssPrintf("Sending to IP address: %s %d\n", LOCALHOST, port);

    // connect
    if (connect(Socket, (struct sockaddr*)&Addr, sizeof(Addr)) != 0) {
        ssPrintf("Connection with server failed...\n");
        ssSetErrorStatus(S, "Connection with server failed...\n");
        return 0;
    }
	
	return 1;
}

/* SendData - Returns 1 upon success. 
 * @SendBuffer: The buffer with data to send.  
 * Description: Sends data to Python server.   
 */
int SendData(SimStruct *S, void * SendBuffer)
{
    int sendAddrSize = sizeof(Addr);
    int sendStatus;
    char error_message[80];
    
	sendStatus = send(Socket, (char *)SendBuffer, SEND_SIZE, 0);
    if (sendStatus == SOCKET_ERROR) {
        sprintf(error_message, "send() failed, error code %d.\n", WSAGetLastError());
        ssPrintf(error_message);
        ssSetErrorStatus(S, error_message);
	} else if (sendStatus != SEND_SIZE) {
        sprintf(error_message, "send() only sent %d bytes \n", sendStatus);
        ssPrintf(error_message);
        ssSetErrorStatus(S, error_message);
    } else {
        return 0;
    }
    
    return sendStatus;
}

/* RecvData - Returns 1 upon success. 
 * @RecvBuffer: The buffer with data to receive.  
 * Description: Receives data from Python server.   
 */
int RecvData(SimStruct *S, void * RecvBuffer)
{    
	int readStatus;
	int recvAddrSize = sizeof(Addr);
    char error_message[80];

	readStatus = recv(Socket, (char *)RecvBuffer, RECV_SIZE, 0);
	if ( readStatus == SOCKET_ERROR ){
        sprintf(error_message, "recv() failed: %d.\n", WSAGetLastError());
        ssPrintf(error_message);
		ssSetErrorStatus(S, error_message);
	} else if (readStatus != RECV_SIZE) {
        sprintf(error_message, "recv() only received %d bytes \n", readStatus);
        ssPrintf(error_message);
        ssSetErrorStatus(S, error_message);
    } else {
        return 0;
    }

	return readStatus;
}

/*====================*
 * S-function methods *
 *====================*/

/* Calls this function during initialization to inquire about ports, states, etc.
 * See sfuntmpl_doc.c for more details on the macros below.
 * @S: SimStruct data structure which Simulink uses to maintain info about the S-Function
 */
static void mdlInitializeSizes(SimStruct *S)
{

    // parameters into s-function 
    ssSetNumSFcnParams(S, N_PARAMS); 
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
        return;
    }
    
    int i;
    for (i = 0; i < N_PARAMS; i++)
        ssSetSFcnParamTunable(S, i, 0);
    
    // continous/discrete states
    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, 0);

    // INPUT PORTS 
    if (!ssSetNumInputPorts(S, 1)) return;
    ssSetInputPortWidth(S, 0, N_INPUT);
    ssSetInputPortRequiredContiguous(S, 0, true);
    ssSetInputPortDirectFeedThrough(S, 0, 1);

    // OUTPUT PORTS
    if (!ssSetNumOutputPorts(S, 1)) return;
    ssSetOutputPortWidth(S, 0, N_OUTPUT); 

    // OTHER
    ssSetNumSampleTimes(S, 1);
    ssSetNumRWork(S, 0);
    ssSetNumIWork(S, 0);
    ssSetNumPWork(S, 0);
    ssSetNumModes(S, 0);
    ssSetNumNonsampledZCs(S, 0);

    // specifying these options together with exception-free code speeds up execution of s-function
    //ssSetOptions(S, SS_OPTION_WORKS_WITH_CODE_REUSE | SS_OPTION_EXCEPTION_FREE_CODE | SS_OPTION_USE_TLC_WITH_ACCELERATOR);
    ssSetOptions(S, 0);
}

/* Calls this function during initialization to set the sample times of the S-function.
 * @S: SimStruct data structure which Simulink uses to maintain info about the S-Function
 */
static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
    ssSetOffsetTime(S, 0, FIXED_IN_MINOR_STEP_OFFSET);
    ssSetModelReferenceSampleTimeDefaultInheritance(S);
}

/* Calls this function during initialization.
 * @S: SimStruct data structure which Simulink uses to maintain info about the S-Function
 */
#define MDL_START
static void mdlStart(SimStruct *S)
{    

    // get port number
    char cwd[10240];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        ssPrintf("Current working dir: %s\n", cwd);
    }

    int port = 9999;
    FILE* ptr = fopen("port.txt", "r");
    if (ptr == NULL) {
        ssPrintf("port.txt does not exist, using port 9999.");
    } else {
        fscanf(ptr, "%d", &port);
    }
    fclose(ptr);
   
    // setup socket
	SetupSocket(S, port);
}

/* Calls this function at each time step to calculate the block outputs.
 * @S: SimStruct data structure which Simulink uses to maintain info about the S-Function
 * @tid: thread id 
 */
static void mdlOutputs(SimStruct *S, int_T tid)
{

    // ptrs to input arrays
    const real_T * u1  = (const real_T*) ssGetInputPortSignal(S,0);
    
    // ptrs to output arrays
    real_T * y1  = (real_T *)ssGetOutputPortRealSignal(S,0);
	
	// buffers
	char SendBuffer[SEND_SIZE];
    double * outData = (double *) SendBuffer;

    char RecvBuffer[RECV_SIZE];
    double * inData = (double *) RecvBuffer;

	// send data 
    unsigned int i;
	for (i=0; i<N_INPUT; i++){
		outData[i] = u1[i];
	}
	SendData(S, SendBuffer);

    // read data
	RecvData(S, RecvBuffer);
	for (i=0; i<N_OUTPUT; i++) {
		y1[i] = inData[i];
	}
}

/* Calls this function at the end of the simulation.
 * @S: SimStruct data structure which Simulink uses to maintain info about the S-Function
 */
static void mdlTerminate(SimStruct *S)
{
	closesocket(Socket);
	WSACleanup();
}

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
