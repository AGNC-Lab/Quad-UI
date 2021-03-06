#ifndef _H_MPU_THREAD_
#define _H_MPU_THREAD_

	
#include "pevents/pevents.h"	//Includes event handling (https://github.com/NeoSmart/PEvents)
#include <pthread.h>
#include "control/QuatRotEuler.h"
#include "threads/stateMachine.h"
#include "control/MatricesAndVectors.h"
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

#define PI 3.1415

using std::string;
using std::ostringstream;
using namespace std;
using namespace neosmart;

extern pthread_mutex_t stateMachine_Mutex;
extern pthread_mutex_t IMU_Mutex;	//protect IMU data

extern Vec3 IMU_Data_RPY;
extern Vec4 IMU_Data_Quat, IMU_Data_QuatNoYaw;
extern Vec3 IMU_Data_Accel;
extern Vec3 IMU_Data_AngVel;

extern neosmart_event_t e_IMU_trigger;
extern neosmart_event_t e_Timeout; //Always false event for forcing timeout of WaitForEvent

extern int threadCount;	
extern int currentState;

//Gather MPU data
void *IMU_Task(void *threadID);

//Timer
void *IMU_Timer(void *threadID);

//get accelerometer calibration parameters
void AccCalibParam(Vec3 *AccCalib, double *radius);

#endif