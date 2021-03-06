

#include "threads/Ros_threads.h"

Vec3 RPY_Vicon;
Vec4 Quat_vicon;
Vec4 IMU_localData_QuatViconYaw, IMU_localData_QuatNoYaw;
Vec4 Vicon_YawQuat;
qcontrol_defs::PVA localPVA_quadVicon;
int ButtonX = 0;
int ButtonY = 0;
int ButtonA = 0;
int ButtonB = 0;
int ButtonLB = 0;
int ButtonRB = 0;

void handle_mp_joy_msg(const sensor_msgs::Joy& msg){
	float yaw_ctr_pos, yaw_ctr_neg;
	int localCurrentState, localYawSource;
	Vec3 IMU_localData_RPY, IMU_localData_RPY_ViconYaw;

	//Grab attitude estimation
	pthread_mutex_lock(&IMU_Mutex);
		IMU_localData_RPY = IMU_Data_RPY;
	pthread_mutex_unlock(&IMU_Mutex);

	pthread_mutex_lock(&PVA_Vicon_Mutex);
		IMU_localData_RPY_ViconYaw = IMU_Data_RPY_ViconYaw;
		localPVA_quadVicon = PVA_quadVicon;
	pthread_mutex_unlock(&PVA_Vicon_Mutex);

	pthread_mutex_lock(&stateMachine_Mutex);
		localCurrentState = currentState;
	pthread_mutex_unlock(&stateMachine_Mutex);


	//If in attitude mode
	if(localCurrentState == ATTITUDE_MODE){
		//Set thrust
		pthread_mutex_lock(&ThrustJoy_Mutex);
			ThrustJoy = msg.axes[1] * maxThrust_AttMode;
		pthread_mutex_unlock(&ThrustJoy_Mutex);

		pthread_mutex_lock(&YawSource_Mutex);
			localYawSource = YawSource;
		pthread_mutex_unlock(&YawSource_Mutex);

		//Set references
		pthread_mutex_lock(&attRefJoy_Mutex);	
			attRefJoy.v[0] = -msg.axes[3]*PI/6; //roll
			attRefJoy.v[1] = msg.axes[4]*PI/6; //pitch
		pthread_mutex_unlock(&attRefJoy_Mutex);
	}
	else if(localCurrentState == MOTOR_MODE){ 	//If in motor mode
		//Set thrust
		pthread_mutex_lock(&ThrustJoy_Mutex);
			ThrustJoy = msg.axes[1] * maxThrust_MotorMode;
		pthread_mutex_unlock(&ThrustJoy_Mutex);

		//Set attitude with zero error
		pthread_mutex_lock(&attRefJoy_Mutex);	
			attRefJoy.v[0] = IMU_localData_RPY.v[0]; //Set ref to actual IMU value
			attRefJoy.v[1] = IMU_localData_RPY.v[1]; //Set ref to actual IMU value
			attRefJoy.v[2] = IMU_localData_RPY.v[2]; //Set ref to actual IMU value
		pthread_mutex_unlock(&attRefJoy_Mutex);
	}
	else{ //If not in a flight mode, set everything to zero
		pthread_mutex_lock(&ThrustJoy_Mutex);
			ThrustJoy = 0;
		pthread_mutex_unlock(&ThrustJoy_Mutex);

		//Set attitude with zero error
		pthread_mutex_lock(&attRefJoy_Mutex);	
			attRefJoy.v[0] = IMU_localData_RPY.v[0]; //Set ref to actual IMU value
			attRefJoy.v[1] = IMU_localData_RPY.v[1]; //Set ref to actual IMU value
			attRefJoy.v[2] = IMU_localData_RPY.v[2]; //Set ref to actual IMU value
		pthread_mutex_unlock(&attRefJoy_Mutex);
	}

	if(localCurrentState == POSITION_JOY_MODE){
		pthread_mutex_lock(&ThrustJoy_Mutex);
			ThrustJoy = msg.axes[1] * maxThrust_AttMode;
		pthread_mutex_unlock(&ThrustJoy_Mutex);
		pthread_mutex_lock(&posRefJoy_Mutex);	
			PVA_RefJoy.pos.position.x += msg.axes[4]*maxVel_PosMode/20; //20hz
			PVA_RefJoy.pos.position.y += msg.axes[3]*maxVel_PosMode/20;
			PVA_RefJoy.pos.position.z += (msg.buttons[5]-msg.buttons[4])*maxVel_PosMode/20;
			PVA_RefJoy.vel.linear.x = msg.axes[4]*maxVel_PosMode;
			PVA_RefJoy.vel.linear.y = msg.axes[3]*maxVel_PosMode;
			PVA_RefJoy.vel.linear.z = (msg.buttons[5]-msg.buttons[4])*maxVel_PosMode;
	  	pthread_mutex_unlock(&posRefJoy_Mutex);	
	}
	else{
			PVA_RefJoy.pos.position.x = PVA_quadVicon.pos.position.x; //20hz
			PVA_RefJoy.pos.position.y = PVA_quadVicon.pos.position.y;
			PVA_RefJoy.pos.position.z = PVA_quadVicon.pos.position.z;
			PVA_RefJoy.vel.linear.x = 0;
			PVA_RefJoy.vel.linear.y = 0;
			PVA_RefJoy.vel.linear.z = 0;
	}

	//Manage yaw reference in attitude and position control modes
	if((localCurrentState == ATTITUDE_MODE) || (localCurrentState == ATTITUDE_MODE)){
		pthread_mutex_lock(&attRefJoy_Mutex);	
			yaw_ctr_pos = msg.axes[2];
			yaw_ctr_neg = msg.axes[5];
			//Set yaw to measured yaw if quad isnt flying
			if (msg.axes[1] <= 0) {
				if(localYawSource == _IMU){
					attRefJoy.v[2] = IMU_localData_RPY.v[2];
				}
				else if (localYawSource == _VICON){
					attRefJoy.v[2] = IMU_localData_RPY_ViconYaw.v[2];
				}
		    }
		    else{ //If quad is flying, increment yaw
		    	if (yaw_ctr_neg < 0) {
					attRefJoy.v[2] -= yaw_Inc;
				}								//yaw
				if (yaw_ctr_pos < 0) {
					attRefJoy.v[2] += yaw_Inc;
				}
		    }
		pthread_mutex_unlock(&attRefJoy_Mutex);
	}
	
	//Compare joystick buttons with previously read (check if state changed)
	if (msg.buttons[0] && !ButtonA){
		SetEvent(e_buttonA);
		//printf("ButtonA Pushed!\n");
	}
	if (msg.buttons[1] && !ButtonB){
		SetEvent(e_buttonB);
		printf("ButtonB Pushed!\n");
	}
	if (msg.buttons[2] && !ButtonX){
		SetEvent(e_buttonX);
		//printf("ButtonX Pushed!\n");
	}
	if (msg.buttons[3] && !ButtonY){
		SetEvent(e_buttonY);
		//printf("ButtonY Pushed!\n");
	}
	// if (msg.buttons[4] && !ButtonLB){
	// 	SetEvent(e_ButtonLB);
	// 	//printf("ButtonY Pushed!\n");
	// }
	// if (msg.buttons[5] && !ButtonRB){
	// 	SetEvent(e_ButtonRB);
	// 	//printf("ButtonY Pushed!\n");
	// }
	ButtonA = msg.buttons[0];
	ButtonB = msg.buttons[1];
	ButtonX = msg.buttons[2];
	ButtonY = msg.buttons[3];
	// ButtonLB = msg.buttons[4];
	// ButtonRB = msg.buttons[5];

}

void handle_Vicon(const geometry_msgs::TransformStamped& msg){

 	pthread_mutex_lock(&PVA_Vicon_Mutex);	
		PVA_quadVicon.pos.position.x = msg.transform.translation.x;
		PVA_quadVicon.pos.position.y = msg.transform.translation.y;
		PVA_quadVicon.pos.position.z = msg.transform.translation.z;

		PVA_quadVicon.t = msg.header.stamp;

		PVA_quadVicon.pos.orientation.w = msg.transform.rotation.w;
		PVA_quadVicon.pos.orientation.x = msg.transform.rotation.x;
		PVA_quadVicon.pos.orientation.y = msg.transform.rotation.y;
		PVA_quadVicon.pos.orientation.z = msg.transform.rotation.z;
  	pthread_mutex_unlock(&PVA_Vicon_Mutex);

  	//Get yaw from vicon measurement and include it into measured quaternion
  	Quat_vicon.v[0] = msg.transform.rotation.w;
  	Quat_vicon.v[1] = msg.transform.rotation.x;
  	Quat_vicon.v[2] = msg.transform.rotation.y;
  	Quat_vicon.v[3] = msg.transform.rotation.z;
  	RPY_Vicon = Quat2RPY(Quat_vicon);

	Vicon_YawQuat.v[0] = cos(RPY_Vicon.v[2]/2);
	Vicon_YawQuat.v[1] = 0;
	Vicon_YawQuat.v[2] = 0;
	Vicon_YawQuat.v[3] = sin(RPY_Vicon.v[2]/2);

	pthread_mutex_lock(&IMU_Mutex);
		IMU_localData_QuatNoYaw = IMU_Data_QuatNoYaw;
	pthread_mutex_unlock(&IMU_Mutex);

	IMU_localData_QuatViconYaw = QuaternionProduct(Vicon_YawQuat, IMU_localData_QuatNoYaw);

	pthread_mutex_lock(&PVA_Vicon_Mutex);
		IMU_Data_Quat_ViconYaw = IMU_localData_QuatViconYaw;
		IMU_Data_RPY_ViconYaw = Quat2RPY(IMU_localData_QuatViconYaw);
	pthread_mutex_unlock(&PVA_Vicon_Mutex);
  	// kalman_v << kalman_state(3,0) << "," << kalman_state(4,0) << "," << kalman_state(5,0) << "\n";
  	// vicon_p << z(0,0) << "," << z(1,0) << "," << z(2,0) << "\n";

}