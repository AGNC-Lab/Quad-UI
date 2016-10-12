

#include "control_thread.h"


void *AttControl_Timer(void *threadID){

	printf("AttControl_Timer has started!\n");
	int SamplingTime = 5;	//Sampling time in milliseconds
	int localCurrentState;

	while(1){
		WaitForEvent(e_Timeout,SamplingTime);

		//Check system state
		pthread_mutex_lock(&stateMachine_Mutex);
			localCurrentState = currentState;
		pthread_mutex_unlock(&stateMachine_Mutex);

		//check if system should be terminated
		if(localCurrentState == TERMINATE){
			break;
		}

		SetEvent(e_AttControl_trigger);
	}
	
	printf("AttControl_Timer stopping...\n");
	//Shutdown here
	threadCount -= 1;
	pthread_exit(NULL);
}


void *AttControl_Task(void *threadID){
	printf("AttControl_Task has started!\n");

	//Initialize PIDs (sets initial errors to zero)
	pthread_mutex_lock(&PID_Mutex);
	initializePID(&PID_att);
	initializePID(&PID_angVel);
	pthread_mutex_unlock(&PID_Mutex);
	float dt = 0.005; 			//Sampling time
	float takeOffThrust = 0.3; //Minimum thrust for resetting integral control
	Vec3 localAttRef;

	//Vectors of zeros
	Vec3 zeros;
	zeros.v[0] = 0; zeros.v[1] = 0; zeros.v[2] = 0; 
	Vec3 feedforward = zeros;

	Vec4 IMU_localData_Quat;
	Vec3 IMU_localData_Vel;
	Vec3 IMU_localData_RPY;
	Vec3 inputTorque;
	Vec4 PCA_localData;
	float localThrust;
	int localCurrentState;
	int localYawSource;


	Vec3 error_att;
	Vec3 error_att_vel;

	Vec3 wDes;
	Mat3x3 Rdes;
	Mat3x3 Rbw;
	

	//Mat3x3 Rdes = RPY2Rot(0,0,0);
	//PrintMat3x3(Concatenate3Vec3_2_Mat3x3(PID_att.K_p, PID_att.K_d, PID_att.K_i));
	//PrintMat3x3(Concatenate3Vec3_2_Mat3x3(PID_angVel.K_p, PID_angVel.K_d, PID_angVel.K_i));

	while(1){

		WaitForEvent(e_AttControl_trigger,500);

		//Check system state
		pthread_mutex_lock(&stateMachine_Mutex);
			localCurrentState = currentState;
		pthread_mutex_unlock(&stateMachine_Mutex);

		//check if system should be terminated
		if(localCurrentState == TERMINATE){
			break;
		}

		//Throttle
	    pthread_mutex_lock(&ThrustJoy_Mutex);
			localThrust = ThrustJoy;
		pthread_mutex_unlock(&ThrustJoy_Mutex);

		//Grab attitude reference
		if(localCurrentState == ATTITUDE_MODE){
			pthread_mutex_lock(&attRefJoy_Mutex);	
			    localAttRef = attRefJoy;
		    pthread_mutex_unlock(&attRefJoy_Mutex);
		    Rdes = RPY2Rot(localAttRef.v[0], localAttRef.v[1], localAttRef.v[2]);
		}
		else if (localCurrentState == POSITION_JOY_MODE){
			pthread_mutex_lock(&attRefPosControl_Mutex);
				Rdes = Rdes_PosControl;
			pthread_mutex_unlock(&attRefPosControl_Mutex);
		}


		pthread_mutex_lock(&YawSource_Mutex);
			localYawSource = YawSource;
		pthread_mutex_unlock(&YawSource_Mutex);

		//Grab attitude estimation
		if (localYawSource == _IMU){
			pthread_mutex_lock(&IMU_Mutex);
				IMU_localData_Quat = IMU_Data_Quat;
				IMU_localData_Vel = IMU_Data_AngVel;
				IMU_localData_RPY = IMU_Data_RPY;
			pthread_mutex_unlock(&IMU_Mutex);
		}
		else{
			pthread_mutex_lock(&PVA_Vicon_Mutex);
				IMU_localData_Quat = IMU_Data_Quat_ViconYaw;
			pthread_mutex_unlock(&PVA_Vicon_Mutex);
			pthread_mutex_lock(&IMU_Mutex);
				IMU_localData_Vel = IMU_Data_AngVel;
				IMU_localData_RPY = IMU_Data_RPY;
			pthread_mutex_unlock(&IMU_Mutex);
		}

		Rbw = Quat2rot(IMU_localData_Quat);

	    //Calculate attitude error
	    error_att = AttitudeErrorVector(Rbw, Rdes);
	    //PrintVec3(error_att, "error_att"); 

		//Update PID
		pthread_mutex_lock(&PID_Mutex);
			if(!isNanVec3(error_att)){
				updateErrorPID(&PID_att, feedforward, error_att, zeros, dt);
			}

			//Dont integrate integrator if not in minimum thrust
			if (localThrust < takeOffThrust){
				resetIntegralErrorPID(&PID_att);
			}
			
			//Reference for inner loop (angular velocity control)
			wDes = outputPID(PID_att);

			//Calculate angular velocity error and update PID
			error_att_vel = Subtract3x1Vec(wDes, IMU_localData_Vel);
			updateErrorPID(&PID_angVel, feedforward, error_att_vel, zeros, dt);

			if (localThrust < takeOffThrust){
				resetIntegralErrorPID(&PID_angVel);
			}

			//This scale of 16.0 should be excluded eventually (incorporate it in gains)
			inputTorque = ScaleVec3(outputPID(PID_angVel), 1.0/16.0);
		pthread_mutex_unlock(&PID_Mutex);

		//Distribute power to motors
		pthread_mutex_lock(&Contr_Input_Mutex);
			Contr_Input.v[0] = localThrust;
			Contr_Input.v[1] = inputTorque.v[0];
			Contr_Input.v[2] = inputTorque.v[1];
			Contr_Input.v[3] = inputTorque.v[2];
			PCA_localData = u2pwmXshape(Contr_Input);
		pthread_mutex_unlock(&Contr_Input_Mutex);

		//Send motor commands
		pthread_mutex_lock(&PCA_Mutex);
			PCA_Data = PCA_localData;
		pthread_mutex_unlock(&PCA_Mutex);


	}
	
	printf("AttControl_Task stopping...\n");
	threadCount -= 1;
	pthread_exit(NULL);
}

void *PosControl_Timer(void *threadID){

	printf("PosControl_Timer has started!\n");
	int SamplingTime = 20;	//Sampling time in milliseconds
	int localCurrentState;

	while(1){
		WaitForEvent(e_Timeout,SamplingTime);

		//Check system state
		pthread_mutex_lock(&stateMachine_Mutex);
			localCurrentState = currentState;
		pthread_mutex_unlock(&stateMachine_Mutex);

		//check if system should be terminated
		if(localCurrentState == TERMINATE){
			break;
		}

		SetEvent(e_PosControl_trigger);
	}
	
	printf("PosControl_Timer stopping...\n");
	//Shutdown here
	threadCount -= 1;
	pthread_exit(NULL);
}

void *PosControl_Task(void *threadID){
	printf("PosControl_Task has started!\n");

	float dt = 0.050; 			//Sampling time
	double m = 0.26, gz = 9.81;  //Mass and gravity for quadcopter
	double nominalThrust = 1.0;
	int localCurrentState;
	float yawDesired;
	Vec3 e_Pos, e_Vel; 			//error in position and velocity
	Vec3 acc_Ref;				//Desired acceleration
	Vec3 feedForward;			//Feedforward vector
	Vec3 Fdes;
	Vec3 z_bdes, x_cdes, y_bdes, x_bdes;
	Vec4 IMU_localData_Quat;
	qcontrol_defs::PVA localPVAEst_quadVicon, localPVA_quadVicon, localPVA_Ref;

	Vec3 z_w;	//z vector of the inertial frame
	Vec3 z_b;	//z vector of the body frame
	z_w.v[0] = 0; z_w.v[1] = 0; z_w.v[2] = 1;

	//Vectors of zeros
	Vec3 zeros;
	zeros.v[0] = 0; zeros.v[1] = 0; zeros.v[2] = 0; 

	//Initialize PIDs (sets initial errors to zero)
	pthread_mutex_lock(&PID_Mutex);
	initializePID(&PID_pos);
	pthread_mutex_unlock(&PID_Mutex);

	while(1){

		WaitForEvent(e_PosControl_trigger,500);

		//Check system state
		pthread_mutex_lock(&stateMachine_Mutex);
			localCurrentState = currentState;
		pthread_mutex_unlock(&stateMachine_Mutex);

		//check if system should be terminated
		if(localCurrentState == TERMINATE){
			break;
		}

		//Grab attitude estimation
		// pthread_mutex_lock(&IMU_Mutex);
		// 	IMU_localData_Quat = IMU_Data_Quat;
		// pthread_mutex_unlock(&IMU_Mutex);
		pthread_mutex_lock(&PVA_Vicon_Mutex);
			IMU_localData_Quat = IMU_Data_Quat_ViconYaw;
		pthread_mutex_unlock(&PVA_Vicon_Mutex);

		//Grab position and velocity estimation
		pthread_mutex_lock(&PVA_Kalman_Mutex);
			localPVAEst_quadVicon = PVA_quadKalman;
		pthread_mutex_unlock(&PVA_Kalman_Mutex);
	 	pthread_mutex_lock(&PVA_Vicon_Mutex);
	 		localPVA_quadVicon = PVA_quadVicon;
	  	pthread_mutex_unlock(&PVA_Vicon_Mutex);
		// pthread_mutex_lock(&PVA_Vicon_Mutex);	
		// 	localPVA_quadVicon = PVA_quadVicon;
	 //  	pthread_mutex_unlock(&PVA_Vicon_Mutex);	

	  	//Grab joystick position and velocity reference
		pthread_mutex_lock(&posRefJoy_Mutex);	
			localPVA_Ref = PVA_RefJoy;
	  	pthread_mutex_unlock(&posRefJoy_Mutex);	

		//Grab yaw reference
		pthread_mutex_lock(&attRefJoy_Mutex);	
		    yawDesired = attRefJoy.v[2];
	    pthread_mutex_unlock(&attRefJoy_Mutex);

	  	//Calculate position and velocity errors
	  	e_Pos.v[0] = localPVA_Ref.pos.position.x - localPVA_quadVicon.pos.position.x;
	  	e_Pos.v[1] = localPVA_Ref.pos.position.y - localPVA_quadVicon.pos.position.y;
	  	e_Pos.v[2] = localPVA_Ref.pos.position.z - localPVA_quadVicon.pos.position.z;
	  	e_Vel.v[0] = localPVA_Ref.vel.linear.x - localPVAEst_quadVicon.vel.linear.x;
	  	e_Vel.v[1] = localPVA_Ref.vel.linear.y - localPVAEst_quadVicon.vel.linear.y;
	  	e_Vel.v[2] = localPVA_Ref.vel.linear.z - localPVAEst_quadVicon.vel.linear.z;
	  	
	  	//Get feedforward vector
	  	acc_Ref.v[0] = localPVA_Ref.acc.linear.x;
	  	acc_Ref.v[1] = localPVA_Ref.acc.linear.y;
	  	acc_Ref.v[2] = localPVA_Ref.acc.linear.z;
	  	feedForward = ScaleVec3(z_w, nominalThrust);
	  	// feedForward = Add3x1Vec(ScaleVec3(z_w, nominalThrust), ScaleVec3(acc_Ref, 1.0/gz));//feedForward = m*gz*z_w + m*ref_dotdot

		//Vehicle attitude
		Mat3x3 Rbw = Quat2rot(IMU_localData_Quat);
		z_b = MultiplyMat3x3Vec3(Rbw, z_w); //z vector of the vehicle in inertial frame

		//Update data in PID, calculate results
		pthread_mutex_lock(&PID_Mutex);
			updateErrorPID(&PID_pos, feedForward, e_Pos, e_Vel, dt);

			//Calculate 3d world desired force for the quadcopter and normalize it
			Fdes = outputPID(PID_pos);
		pthread_mutex_unlock(&PID_Mutex);

		//Desired thrust in body frame
	    pthread_mutex_lock(&ThrustPosControl_Mutex);
			ThrustPosControl = innerProd(Fdes, z_b);
		pthread_mutex_unlock(&ThrustPosControl_Mutex);

		//Find desired attitude from desired force and yaw angle
		z_bdes = normalizeVec3(Fdes);
		// x_cdes.v[0] = cos(yawDesired); 
		// x_cdes.v[1] = sin(yawDesired); 
		x_cdes.v[0] = 1; 
		x_cdes.v[1] = 0; 
		x_cdes.v[2] = 0;
		y_bdes = normalizeVec3(cross(z_bdes, x_cdes));
		x_bdes = cross(y_bdes, z_bdes);

		//Set desired rotation matrix for attitude
		pthread_mutex_lock(&attRefPosControl_Mutex);
			Rdes_PosControl = Concatenate3Vec3_2_Mat3x3(x_bdes, y_bdes, z_bdes);
		pthread_mutex_unlock(&attRefPosControl_Mutex);

		// PrintMat3x3(Rdes);


	}
	
	printf("PosControl_Task stopping...\n");
	threadCount -= 1;
	pthread_exit(NULL);
}