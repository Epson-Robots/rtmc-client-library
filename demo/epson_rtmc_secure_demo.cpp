/*
  Copyright 2025 Seiko Epson Corporation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
*/
#include "epson_rtmc_demo.h"
#include "rtmc/rtmc_client.h"
#include "rtmc/rtmc_definition.h"

#define DEMO_AMPLITUDE 0.001
#define DEMO_PERIOD 1000

#define DEMO_JOINT_MAX 6
#define DEMO_IPADDRESS "192.168.0.1"
#define DEMO_RC_BUILTINMSG_PORT 7000

using namespace epson_rtmc_client;

int main(int argc, const char** argv)
{
   cout<<"Starting epson_rtmc_secure_demo"<<endl;
  string password;
  string rb_model;
  if( (argc > 1) && (argc < 4) ){
    if(argc == 3) 
    {
      password=argv[2];
      rb_model=argv[1];
    }else{
      rb_model=argv[1];
    }
  }else{
    cout<<"Invalid argument"<<endl;
    return 1;
  }

  Create_Ref create_ref(DEMO_AMPLITUDE,DEMO_PERIOD);
  COMPROPERTY com_property={"",0};
  SECPROPERTY sec_property;
  vector<double> current_ja;
  vector<double> rtmc_ja_ref;
  RCSTATUS rc_status= {0,0,0,0,0,{0,0,0,""},{0,0,0,""}};

  double weight=0.0;
  double inertia=0.0;
  unsigned short eccentricity=0;
  

  current_ja.assign(DEMO_JOINT_MAX,0.0);
  rtmc_ja_ref.assign(DEMO_JOINT_MAX,0.0);

  RTMCClient user_client;
  
  try{
  short ret=0;
  unsigned short buff_size=10;
  unsigned short cnt=0;

  //Update the configuration when epson robot controller's IP address or built-in message port number is changed
  com_property.rc_ip_address=DEMO_IPADDRESS;
  com_property.rc_builtinmsg_port=DEMO_RC_BUILTINMSG_PORT;

  ret=user_client.Set_ComProperty(com_property);
  if(ret != CLIENT_SUCCESS) 
  {
    cout<< "Failed to execute Set_Comproperty function"<<endl;
    throw ret;
  }

  // To enable secure communicaiton, set "security_communication_flag" to true
  sec_property.security_communication_flag=true;
  // Specify the path to the CA certificate
  sec_property.ca_cert_filepath="../client_cert/ca_cert_example.pem";
  // Specify the path to signed certificate for Linux PC application
  sec_property.client_cert_filepath="../client_cert/client_cert_example.pem";
  //Specify the path to private key for Linux PC application
  sec_property.client_private_key="../client_cert/client_private_key_example.pem";
  //Speciyf the path to governance document for Linux PC application
  sec_property.client_governance_filepath="../client_cert/client_governance_example.smime";
  //Speciyf the path to permissions document for Linux PC application
  sec_property.client_permissions_filepath="../client_cert/client_permissions_example.smime";
  
  // Apply the security setting
  ret=user_client.Set_SecProperty(sec_property);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Set_SecProperty function"<<endl;
    throw ret;
  }
  // Enter the password to start communication with epson robot controller
  ret=user_client.RTMC_Connect(password);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute RTMC_Connect function"<<endl;
    throw ret;
  }
  
  // Select whether to control only the robot or both the robot and standard I/O output
  // RTMC_SEND_FORMAT_RB is defined as "0"
  ret=user_client.Set_RTMCSendFormat(RTMC_SEND_FORMAT_RB);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Set_RTMCSendFormat function"<<endl;
    throw ret;
  }

  // Indicate whether the target status is for the robot or for the robot + standard I/O input
  // RTMC_RECV_FORMAT_RB is defined as "0"
  ret=user_client.Set_RTMCRecvFormat(RTMC_RECV_FORMAT_RB);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Set_RTMCRecvFormat function"<<endl;
    throw ret;
  }
  
  // Set the buffer size used for robot control
  ret=user_client.Set_BufferSize(buff_size);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Set_BufferSize function"<<endl;
    throw ret;
  }

  // Verify the model match before issuing command to the robot
  ret=user_client.RBCheck(rb_model);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute RBCheck function"<<endl;
    throw ret;
  }

  //Get Weight parameter set in epson robot controller
  ret=user_client.Get_Weight(&weight);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Get_Weight function"<<endl;
    throw ret;
  }

  
  //Set weight parameter according to the weight at the epson robot's end-effector 
  ret=user_client.Set_Weight(weight);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Set_Weight function"<<endl;
    throw ret;
  }

  //Get Inertia parameter and eccentricity parameter in epson robot controller
  ret=user_client.Get_Inertia(&inertia,&eccentricity);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Get_Inertia function"<<endl;
    throw ret;
  }

  //Set inertia parameter and eccentricity parameter according to the inertia and eccentricity at the epson robot's end-effector
  ret=user_client.Set_Inertia(inertia,eccentricity);
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Set_Inertia function"<<endl;
    throw ret;
  }

  // Reset the error state and related condition
  ret=user_client.Reset();
  if(ret != CLIENT_SUCCESS)
  {
      cout<< "Failed to execute Reset function"<<endl;
      throw ret;
  }

  //Waiting for the error to reset
  do
  {
    //Retrive the controller's error status, safety guard, and emergency stop
    ret=user_client.Get_RCStatus(rc_status);
    if(ret != CLIENT_SUCCESS)
    {
      cout<< "Failed to execute Get_RCStatus function"<<endl;
      throw ret;
    }
    usleep(100000);
    cnt++;

    if(cnt==10)
    {
      cnt=0;
      if(rc_status.err_num > RC_NONERR) throw (short) CLIENT_RCERR_STATUS_ERR;
      if(rc_status.eStop == RC_ESTOP_ON) throw (short) CLIENT_RCESTOP_STATUS_ERR;
      if(rc_status.safeGuard == RC_SG_OPEN) throw (short) CLIENT_RCSG_STATUS_ERR; 
    }

  }while(rc_status.err_num > RC_NONERR || rc_status.eStop==RC_ESTOP_ON || rc_status.safeGuard == RC_SG_OPEN);

  //Turn the motor ON
  ret=user_client.MotorOn();
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute MotorOn function"<<endl;
    throw ret;
  }

  // Turn the power HIGH
  ret=user_client.PowerHigh();
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute PowerHigh function"<<endl;
    throw ret;
  }

  // Get the current joint position
  ret=user_client.Get_CurrentJA(current_ja);
  if(ret != CLIENT_SUCCESS)
  {
     cout<< "Failed to execute Get_CurrentJA function"<<endl;
    throw ret;
  }

  rtmc_ja_ref=current_ja;
  
  // Turn the RT Motion Control mode
  ret=user_client.Set_RTMCModeEnable();
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Set_RTMCModeEnable function"<<endl;
  }

  for(int i=0;i<=(DEMO_PERIOD+buff_size);i++)
  {
    try
    {
      // Update the command value for each joint
      create_ref.Create_JARef(rtmc_ja_ref,i);

      //Execute robot motion based on the command valuse for each joint
      ret=user_client.Exec_RTMC(rtmc_ja_ref,current_ja);

      if(ret != CLIENT_SUCCESS) throw ret;
    }
    catch(short err)
    {
      
      if(err== CLIENT_RCERR_STATUS_ERR ||
         err== CLIENT_RCESTOP_STATUS_ERR ||
         err== CLIENT_RCSG_STATUS_ERR ||
         err== CLIENT_OPERATION_MODE_ERR)
      {
        switch(err)
        {
          case CLIENT_RCERR_STATUS_ERR :  cout<<"An error occurred in epson robot controller"<<endl;  break;
          case CLIENT_RCESTOP_STATUS_ERR :  cout<<"Epson robot controller is in an e-stop condition"<<endl;  break;
          case CLIENT_RCSG_STATUS_ERR :  cout<<"The safe guard has been opened"<<endl;  break;
          case CLIENT_OPERATION_MODE_ERR :  cout<<"The operation mode changed : exited 'Auto' mode"<<endl; break;
        }

        cout<< "Please press the Enter button after sloving the issue that caused the robot motion to fail"; 
        std::cin.ignore();

        // Reset the error state and related condition
        ret=user_client.Reset();
        if(ret != CLIENT_SUCCESS)
        {
          cout<< "Failed to execute Reset function"<<endl;
          throw ret;
        }

        //Waiting for the error to reset
        do
        {
          //Retrive the controller's error status, safety guard, and emergency stop
          ret=user_client.Get_RCStatus(rc_status);
          if(ret != CLIENT_SUCCESS)
          {
            cout<< "Failed to execute Get_RCStatus function"<<endl;
            throw ret;
          }
          usleep(1000);
          cnt++;

          if(cnt==10)
          {
            cnt=0;
            if(rc_status.err_num > RC_NONERR) throw (short) CLIENT_RCERR_STATUS_ERR;
            if(rc_status.eStop == RC_ESTOP_ON) throw (short) CLIENT_RCESTOP_STATUS_ERR;
            if(rc_status.safeGuard == RC_SG_OPEN) throw (short) CLIENT_RCSG_STATUS_ERR; 
          }

        }while(rc_status.err_num > RC_NONERR || rc_status.eStop==RC_ESTOP_ON || rc_status.safeGuard == RC_SG_OPEN);


        //Turn the motor ON
        ret=user_client.MotorOn();
        if(ret != CLIENT_SUCCESS)
        {
          cout<< "Failed to execute MotorOn function"<<endl;
          throw ret;
        }

        // Turn the power HIGH
        ret=user_client.PowerHigh();
        if(ret != CLIENT_SUCCESS)
        {
          cout<< "Failed to execute PowerHigh function"<<endl;
          throw ret;
        }

        // Get the current joint position
        ret=user_client.Get_CurrentJA(current_ja);
        if(ret != CLIENT_SUCCESS)
        {
          cout<< "Failed to execute Get_CurrentJA function"<<endl;
          throw ret;
        }
        rtmc_ja_ref=current_ja;

         // Turn the RT Motion Control mode
        ret=user_client.Set_RTMCModeEnable();
        if(ret != CLIENT_SUCCESS)
        {
          cout<< "Failed to execute Set_RTMCModeEnable function"<<endl;
          throw ret;
        }

        i=0;

        continue;

      }
      else
      {
        throw ret;
      }

    }

  }

  // Exiting RT Motion Control mode
  ret=user_client.Set_RTMCModeDisable();
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute Set_RTMCModeDisable function"<<endl;
    throw ret;
  }

  //Turn the motor OFF
  ret=user_client.MotorOff();
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute MotorOff function"<<endl;
    throw ret;
  }

  // Disconnection from epson robot controller
  ret=user_client.RTMC_DisConnect();
  if(ret != CLIENT_SUCCESS)
  {
    cout<< "Failed to execute RTMC_DisConnect function"<<endl;
    throw ret;
  }

  }
  catch (short errcode)
  {
    switch (errcode)
    {
        case CLIENT_ERR : cout<<"Command execution failed. Please refer to the manual for cause"<<endl; break;

        case CLIENT_OPERATION_MODE_ERR : cout<< "Can not be executed because the operation mode is not set to Auto"<<endl; break;

        case CLIENT_RBCHK_ERR : cout<<"Cannot be executed because the robot model verification has not been completed"<<endl; break;

        case CLIENT_RTMC_MODE_ERR : cout<<"Cannot be executed in the current RT Motion Control mode" <<endl; break;

        case CLIENT_RCERR_STATUS_ERR : cout<<"Cannot be executed due to epson robot controller error"<<endl;  break;

        case CLIENT_RCESTOP_STATUS_ERR : cout<<"Cannot be executed due to an emergency stop condition"<<endl; ; break;

        case CLIENT_RCSG_STATUS_ERR : cout<<"Cannot be executed because the safe guard is open"<<endl; break;

        case CLIENT_COM_ERR : cout<<"Cannot be executed due to a communication error"<<endl; break;

        case CLIENT_TIMEOUT_ERR :  cout<<"Command execution failed due to a timeout"<<endl; break;

        case CLIENT_ARG_ERR : cout<<"Cannot be executed due to invalid argument"<<endl;  break;

        case CLIENT_UNDEF_ERR : cout<<"Command execution failed due to an undefined error"<<endl;  break; 
    }

    // Disconnection from epson robot controller
    user_client.RTMC_DisConnect();

    return errcode;

  }

  cout<<"epson_rtmc_secure_demo has been completed"<<endl;
  return 0;
  
}


