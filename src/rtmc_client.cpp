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
   limitations under the License.
*/

#include "rtmc/DDSCommunication.h"
#include "rtmc/rb_weight_inertia_param.h"
#include "rtmc/dataformat_converter.h"
#include "rtmc/rtmc_log.h"

#include <thread>
#include <bitset>
#include <iostream>
#include <fstream>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <regex>


using namespace std;

namespace epson_rtmc_client
{

    RTMCClient::RTMCClient()
    {
        DDSCommunication_Ptr = nullptr;
        RBWeightInertiaParam_Ptr = nullptr;
        DataFormatConverter_Ptr =nullptr;
        RBWeightInertiaParam_Ptr = new RBWeightInertiaParam();
        DataFormatConverter_Ptr = new DataFormatConverter();

        com_property_option_flag=false;
        com_property_option.client_ip_address="";
        com_property_option.client_builtinmsg_port=0;
        com_property_option.client_userdata_port=0;
        com_property.rc_ip_address="192.168.0.1";
        com_property.rc_builtinmsg_port=7000;
        sec_property.security_communication_flag=false;
        sec_property.ca_cert_filepath="";
        sec_property.client_cert_filepath="";
        sec_property.client_private_key="";
        sec_property.client_governance_filepath="";
        sec_property.client_permissions_filepath="";

        current_ja.assign(RTMC_MAX_JOINT,0.0);
        current_ja.shrink_to_fit();

        rc_status.safeGuard=0;
        rc_status.eStop=0;
        rc_status.operation_mode=0;
        rc_status.err_num=0;
        rc_status.wrn_num=0;
        rc_status.err_info.add_info1=0;
        rc_status.err_info.add_info2=0;
        rc_status.err_info.jnt=0;
        rc_status.err_info.msg="";
        rc_status.wrn_info.add_info1=0;
        rc_status.wrn_info.add_info2=0;
        rc_status.wrn_info.jnt=0;
        rc_status.wrn_info.msg="";

        sendformat=0;
        recvformat=0;

    }

    RTMCClient::~RTMCClient()
    {
        if(DDSCommunication_Ptr != nullptr) DisconnectRC();
        delete RBWeightInertiaParam_Ptr;
        RBWeightInertiaParam_Ptr=nullptr;
        delete DataFormatConverter_Ptr;
        DataFormatConverter_Ptr=nullptr;

    }

    short RTMCClient::Set_ComProperty(const COMPROPERTY& set_com_property, COMPROPERTYOPT set_com_property_option)
    {
        try{

            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            if(!isIPAddress(set_com_property.rc_ip_address))
            {
                RTMCLOG_ERR("A non-IP address string was provided");
                timed_mtx_common_.unlock();
                return CLIENT_ARG_ERR;
            }


            COMPROPERTYOPT default_option={"",0,0};

            if(DDSCommunication_Ptr == nullptr)
            {

                com_property=set_com_property;

                if((set_com_property_option.client_ip_address != default_option.client_ip_address) &&
                (set_com_property_option.client_builtinmsg_port != default_option.client_builtinmsg_port) &&
                (set_com_property_option.client_userdata_port != default_option.client_userdata_port)
                    ){
                        try{

                            if(set_com_property_option.client_builtinmsg_port == set_com_property_option.client_userdata_port) throw (short) CLIENT_ARG_ERR;

                            short ret=Check_ValidIP(set_com_property_option.client_ip_address);
                            if(ret!=CLIENT_SUCCESS) throw ret;

                            ret=Check_ValidPort(set_com_property_option.client_ip_address,set_com_property_option.client_builtinmsg_port);
                            if(ret!=CLIENT_SUCCESS) throw ret;

                            ret=Check_ValidPort(set_com_property_option.client_ip_address,set_com_property_option.client_userdata_port);
                            if(ret!=CLIENT_SUCCESS) throw ret;
                            

                        }
                        catch(short err)
                        {
                            timed_mtx_common_.unlock();
                            return err;
                        }

                        com_property_option = set_com_property_option;
                        com_property_option_flag=true;

                    }
                    else{
                        com_property_option_flag=false;
                    }
                
            }else{
                RTMCLOG_ERR("Cannot be executed in an established connection state");
                timed_mtx_common_.unlock();
                return CLIENT_ERR;
            }
            
            timed_mtx_common_.unlock();
            return CLIENT_SUCCESS;
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_ComProperty(COMPROPERTY* pget_com_property, COMPROPERTYOPT* pget_com_property_option)
    {

        try{

            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            if(DDSCommunication_Ptr == nullptr)
            {

                *pget_com_property=com_property;

                if(pget_com_property_option != nullptr)
                {
                    *pget_com_property_option=com_property_option;
                }

            }else{
                RTMCLOG_ERR("Cannot be executed in an established connection state");
                timed_mtx_common_.unlock();
                return CLIENT_ERR;
            }

            timed_mtx_common_.unlock();
            return CLIENT_SUCCESS;
        }

        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    
    }

    short RTMCClient::Set_SecProperty(const SECPROPERTY& set_sec_property)
    {

        try{

            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            if(DDSCommunication_Ptr == nullptr)
            {   
                sec_property=set_sec_property;
            }else{
                RTMCLOG_ERR("Cannot be executed in an established connection state");
                timed_mtx_common_.unlock();
                return CLIENT_ERR;
            }


            timed_mtx_common_.unlock();
            return CLIENT_SUCCESS;
        }

        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_SecProperty(SECPROPERTY* pget_sec_property)
    {
        try{

            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            if(DDSCommunication_Ptr == nullptr)
            {

                *pget_sec_property=sec_property;
            }else{
                RTMCLOG_ERR("Cannot be executed in an established connection state");
                timed_mtx_common_.unlock();
                return CLIENT_ERR;
            }

            timed_mtx_common_.unlock();
            return CLIENT_SUCCESS;
        }

        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::RTMC_Connect(const string& password)
    {
        try{

        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
        
        RRDCoreData rrd_res={0,0,{}};
        vector<uint8_t> local_add_info;
        short res=0;
        bool ret=false;
        double temp_current_ja=0.0;
        uint8_t FailedDetail=0;
        INTERNALRCSTATUS temp_rc_status={0,0,0,0,0,{0,0,0},{0,0,0}};
        rrd_res.exData.assign(sizeof(double)*6,0);
        local_add_info.assign(password.size(),0);

        if(DDSCommunication_Ptr != nullptr)
        {
            if(!DDSCommunication_Ptr->DDSConnection())
            {   
                DisconnectRC();
            }else{
                RTMCLOG_ERR("The Connection with the controller has been established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
        }

            if(password.size() > 32)
            {
                RTMCLOG_ERR("The password is too long"); 
                timed_mtx_common_.unlock();
                return CLIENT_ARG_ERR;
            }

            DDSCommunication_Ptr = nullptr;
            DDSCommunication_Ptr= new DDSCommunication();
            ret=DDSCommunication_Ptr->init(com_property,com_property_option_flag,com_property_option,sec_property);
            if(!ret)
            {
                DisconnectRC();
                timed_mtx_common_.unlock();
                return CLIENT_UNDEF_ERR;
            }
            try{
                unsigned int wait_time = 3000; // [ms]
                if (!DDSCommunication_Ptr->wait_for_connect(wait_time)) {
                    throw (short) TIMEOUT_ERR;
                }
            }
            catch(short)
            {
                RTMCLOG_ERR("Failed to establish connection due to timeout");
                DisconnectRC();
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }

            DataFormatConverter_Ptr->String2Uint8(password,local_add_info,0);
            res=SendProc_Common(REQ_CONNECT,RTMC_CONNECTION,RTMC_NONDATA,local_add_info,rrd_res);
            if(res!=CLIENT_SUCCESS)
            {
                if(res==CLIENT_ERR) 
                {
                    res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low,&FailedDetail);
                    
                    if(res == CLIENT_ERR)
                    {
                        if(FailedDetail==UNAUTHENTICAITION_PASSWORD)
                        {
                            RTMCLOG_ERR("Password authentication failed");
                        }
                        else if(FailedDetail==RC_OPTION_DISABLE)
                        {
                            RTMCLOG_ERR("Epson robot controller option is not enabled");
                        }
                        else{
                            RTMCLOG_ERR_EX("Undefined error has occurred while establishing the connection",1000,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                            DisconnectRC();
                            timed_mtx_common_.unlock();
                            return  CLIENT_UNDEF_ERR;
                        }

                    }
                    
                    DisconnectRC();
                }

                timed_mtx_common_.unlock();
                return res;
            }

            local_add_info.clear();
            rrd_res.rrd_res_hi=0;
            rrd_res.rrd_res_low=0;
            rrd_res.exData.clear();
            
            res=SendProc_Common(SYS_REQ_CURPOS_INFO,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
            if(res!=CLIENT_SUCCESS) 
            {
                if(res==CLIENT_ERR)
                { 
                    res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
                    DisconnectRC();
                }
                timed_mtx_common_.unlock();
                return res;
            }
            for(unsigned short i=0; i<RTMC_MAX_JOINT; i++) 
            {
                    DataFormatConverter_Ptr->Uint82Double(rrd_res.exData,&temp_current_ja,i*8);
                    current_ja[i]=temp_current_ja;
            }
        
            local_add_info.clear();
            rrd_res.rrd_res_hi=0;
            rrd_res.rrd_res_low=0;
            rrd_res.exData.clear();  
            res=SendProc_Common(SYS_REQ_RC_STATUS,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
            if(res!=CLIENT_SUCCESS) 
            {
                if(res==CLIENT_ERR)
                {
                    res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
                    DisconnectRC();
                }
                timed_mtx_common_.unlock();
                return res;
            }
            
            DataFormatConverter_Ptr->Uint82Rc_status(rrd_res.exData,&temp_rc_status);
            local_add_info.clear(); 
            rrd_res.rrd_res_hi=0;
            rrd_res.rrd_res_low=0;
            rrd_res.exData.clear(); 
            res=Update_RCSts_Core(&rc_status,temp_rc_status);
            if(res != CLIENT_SUCCESS)
            {
                timed_mtx_common_.unlock();
                return res;
            }

            local_add_info.clear(); 
            rrd_res.rrd_res_hi=0;
            rrd_res.rrd_res_low=0;
            rrd_res.exData.clear();   
            res=SendProc_Common(SYS_REQ_RTMC_FORMAT_INFO,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
            if(res!=CLIENT_SUCCESS) 
            {
                if(res==CLIENT_ERR)
                {
                    res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
                    DisconnectRC();
                }
                timed_mtx_common_.unlock();
                return res;
            }
            
            if(((rrd_res.exData[0] != SEND_FORMAT_RB) && (rrd_res.exData[0] != SEND_FORMAT_RBIO))||
            ((rrd_res.exData[1] != RECV_FORMAT_RB) && (rrd_res.exData[1] != RECV_FORMAT_RBIO)))
            {
                RTMCLOG_ERR_EX("Received an undefined format from SYS_REQ_RTMC_FORMAT_INFO",102,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                DisconnectRC();
                timed_mtx_common_.unlock();
                return  CLIENT_UNDEF_ERR;
            }   

            sendformat=rrd_res.exData[0];
            recvformat=rrd_res.exData[1];

        RBWeightInertiaParam_Ptr->Reset_RBModel();
        timed_mtx_common_.unlock();
        return CLIENT_SUCCESS;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::RTMC_DisConnect()
    {
        try{

            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
                res=SendProc_Common(REQ_CONNECT,RTMC_DISCONNECTION,RTMC_NONDATA,local_add_info,rrd_res);
                if(res!=CLIENT_SUCCESS) 
                {
                    if( res == CLIENT_ERR) DisconnectRC();
                    res=CLIENT_ERR;
                    return res;
                }
                
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            DisconnectRC();
            timed_mtx_common_.unlock();
            return res;
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }

    short RTMCClient::RBCheck(const string& rb_model)
    {
        try{

            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(rb_model.size(),0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {

                if(!RBWeightInertiaParam_Ptr->Set_RBModel(rb_model))
                {
                    RTMCLOG_ERR("This robot model is not supported");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }
                
                DataFormatConverter_Ptr->String2Uint8(rb_model,local_add_info,0);
                res=SendProc_Common(REQ_CHECK_ROBOTMODEL,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);    
                if(res==CLIENT_SUCCESS)
                { 
                    if(rrd_res.rrd_res_low == RB_MODEL_UNMATCH)
                    {
                        res=CLIENT_ERR;
                    }else if(rrd_res.rrd_res_low == RB_MODEL_MATCH){
                        ;
                    }else{
                        RTMCLOG_ERR_EX("Undefined error has occurred in RBCheck",1001,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        DisconnectRC();
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }

                }
                
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::RBCheckResult()
    {
        try{

            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
        
                res=SendProc_Common(REQ_GET_ROBOTMODEL_CHECKRESULT,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR)res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
                if(res==CLIENT_SUCCESS){
                    if(rrd_res.rrd_res_low == RB_MODEL_UNMATCH)
                    {
                        res=CLIENT_ERR;
                    }else if(rrd_res.rrd_res_low == RB_MODEL_MATCH)
                    {
                        ;
                    }else{
                        RTMCLOG_ERR_EX("Undefined error has occurred in RBCheckResult",1002,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        DisconnectRC();
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                }

                

            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }

    short RTMCClient::Get_RBModel(string* rb_model)
    {
        try{

            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
        
                res=SendProc_Common(REQ_GET_ROBOTMODEL,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS){
                    if(rrd_res.exData.size()>0){
                        rb_model->resize(rrd_res.exData.size()-1);
                        DataFormatConverter_Ptr->Uint82String(rrd_res.exData,rb_model,0,static_cast<unsigned short>(rrd_res.exData.size()-1));
                    }else{
                        DisconnectRC();
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_RBModel",1003,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                }
                if(res==CLIENT_ERR)   res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);

            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }

    short RTMCClient::Set_RTMCSendFormat(unsigned short set_sendformat)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);
            short res=0;
            uint8_t FailedDetail=0;


            if(DDSCommunication_Ptr!=nullptr)
            {

                if((set_sendformat != SEND_FORMAT_RB && set_sendformat != SEND_FORMAT_RBIO))
                {
                    RTMCLOG_ERR("The argument is out of range");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }
                
                res=SendProc_Common(REQ_SET_RTPOS_IO_FORM,set_sendformat,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) sendformat=set_sendformat;
                if(res==CLIENT_ERR) {
                    res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low,&FailedDetail);
                    if(res == CLIENT_ERR)
                    {
                        if(FailedDetail == ASSIGEND_REMOTEIO)
                        {
                            RTMCLOG_ERR("Remote IO is assigned to standard IO output");
                            
                        }else{
                            RTMCLOG_ERR_EX("Undefined error has occurred in Set_RTMCSendFormat",1004,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                            DisconnectRC();
                            timed_mtx_common_.unlock();
                            return CLIENT_UNDEF_ERR;
                        }
                    }
                }

            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_RTMCSendFormat(unsigned short* get_sendformat)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);


            if(DDSCommunication_Ptr!=nullptr)
            {
        
                res=SendProc_Common(REQ_GET_RTPOS_IO_FORM,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                
                if(res==CLIENT_SUCCESS){
                    if((rrd_res.rrd_res_low != SEND_FORMAT_RB && rrd_res.rrd_res_low != SEND_FORMAT_RBIO)||
                        rrd_res.rrd_res_low != sendformat)
                    {
                        DisconnectRC();
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_RTMCSendFormat",1005,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                    *get_sendformat=rrd_res.rrd_res_low ;
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);

            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }

            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Set_RTMCRecvFormat(unsigned short set_recvformat)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {

                if((set_recvformat != RECV_FORMAT_RB && set_recvformat != RECV_FORMAT_RBIO))
                {
                    RTMCLOG_ERR("The argument is out of range");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }
                
                res=SendProc_Common(REQ_SET_CURPOS_IO_FORM,set_recvformat,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) recvformat=set_recvformat;
                if(res==CLIENT_ERR)  res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);

            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_RTMCRecvFormat(unsigned short* get_recvformat)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
        
                res=SendProc_Common(REQ_GET_CURPOS_IO_FORM,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                
                if(res==CLIENT_SUCCESS){
                    if((rrd_res.rrd_res_low != RECV_FORMAT_RB && rrd_res.rrd_res_low != RECV_FORMAT_RBIO)||
                        rrd_res.rrd_res_low != recvformat)
                    {
                    
                        DisconnectRC();
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_RTMCRecvFormat",1007,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                    *get_recvformat=rrd_res.rrd_res_low ;
                }
                if(res==CLIENT_ERR)  res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);

            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::MotorOn()
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {

                res=SendProc_Common(REQ_SET_MOTOR,RTMC_MOTORON,RTMC_NONDATA ,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);

            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::MotorOff()
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {

                res=SendProc_Common(REQ_SET_MOTOR,RTMC_MOTOROFF,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_MotorStatus(bool* pmotorstatus)
    {
        try{
        
        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
        
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);


            if(DDSCommunication_Ptr!=nullptr)
            {
                res=SendProc_Common(REQ_GET_MOTOR,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS)
                {
                    if(((rrd_res.rrd_res_low != RTMC_MOTORON) && (rrd_res.rrd_res_low != RTMC_MOTOROFF)))
                    {
                        DisconnectRC();
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_MotorStatus",1009,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                    *pmotorstatus=static_cast<bool>(rrd_res.rrd_res_low);
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }

    short RTMCClient::PowerHigh()
    {
        try{
        
        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
        
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);


            if(DDSCommunication_Ptr!=nullptr)
            {

                res=SendProc_Common(REQ_SET_POWER,RTMC_POWERHIGH,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);

            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::PowerLow()
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
                res=SendProc_Common(REQ_SET_POWER,RTMC_POWERLOW,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
                timed_mtx_common_.unlock();
                return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_PowerMode(bool* ppowermode)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
        
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
                res=SendProc_Common(REQ_GET_POWER,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS)
                {
                    if((rrd_res.rrd_res_low != RTMC_POWERHIGH && rrd_res.rrd_res_low != RTMC_POWERLOW))
                    {
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_PowerMode",1011,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        DisconnectRC();
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }

                    *ppowermode=static_cast<bool>(rrd_res.rrd_res_low);
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }

    short RTMCClient::Set_RTMCModeEnable()
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(sizeof(unsigned short),0);
            short res=0;
            uint8_t FailedDetail=0;

            if(DDSCommunication_Ptr!=nullptr)
            {
                
                local_add_info.clear();
                rrd_res.rrd_res_hi=0;
                rrd_res.rrd_res_low=0;
                rrd_res.exData.clear();
                res=SendProc_Common(REQ_SET_ROBOT_CONTROL_MODE,RTMC_MODE_ENABLE,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR)
                {   
                    res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low,&FailedDetail);
                    if(res==CLIENT_ERR)
                    {
                        if(FailedDetail==MOTOR_POWER_OFF) 
                        {
                            RTMCLOG_ERR("The motor power is in the OFF state");
                        }else if(FailedDetail == STOP_PROGRESS){
                            
                            RTMCLOG_ERR("Epson robot is in the process of decelerating");
                        }
                        else{
                            RTMCLOG_ERR_EX("Undefined error has occurred in Set_RTMCModeEnable",1012,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                            DisconnectRC();
                            timed_mtx_common_.unlock();
                            return CLIENT_UNDEF_ERR;
                        }
                    }
                }
                
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Set_RTMCModeDisable()
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
                res=SendProc_Common(REQ_SET_ROBOT_CONTROL_MODE,RTMC_MODE_DISABLE,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_RTMCMode(bool *prtmcmode)
    {
        try{
        
        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
        
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {

                res=SendProc_Common(REQ_GET_ROBOT_CONTROL_MODE,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS)
                {
                    if((rrd_res.rrd_res_low != RTMC_MODE_ENABLE && rrd_res.rrd_res_low != RTMC_MODE_DISABLE))
                    {
                        DisconnectRC();
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_RTMCMode",1013,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }

                    *prtmcmode=static_cast<bool>(rrd_res.rrd_res_low);
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Exec_RTMC(const vector<double>& jacmd,vector<double>& pcurja,uint8_t* pbufstatus)
    {
        try{
        
        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(sizeof(double)*6+sizeof(unsigned short),0);
            short res=0;
            rrd_res.exData.assign(sizeof(uint8_t)+(sizeof(double)*6)+sizeof(unsigned int),0);


            if(DDSCommunication_Ptr!=nullptr)
            {

                if( sendformat != SEND_FORMAT_RB)
                {
                    RTMCLOG_ERR("Missing arguments related standard I/O output");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }

                if(recvformat != RECV_FORMAT_RB)
                {
                    RTMCLOG_ERR("Missing arguments related standard I/O input");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }


                DataFormatConverter_Ptr->Rrdrtpos2Uint8(jacmd,local_add_info);
            
                res=SendProc_Common(REQ_EXEC_RTPOS,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS)
                
                {
                    DataFormatConverter_Ptr->Uint82Rrd_cur_pos(rrd_res.exData,pbufstatus,current_ja);
                    pcurja=current_ja;
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Exec_RTMC(const vector<double>& jacmd,vector<double>& pcurja,unsigned short iocmd,uint8_t* pbufstatus)
    {
        try{
        
        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
        
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(sizeof(double)*6+sizeof(unsigned short),0);
            short res=0;
            rrd_res.exData.assign(sizeof(uint8_t)+(sizeof(double)*6)+sizeof(unsigned int),0);

            if(DDSCommunication_Ptr!=nullptr)
            {
                if( sendformat != SEND_FORMAT_RBIO)
                {
                    RTMCLOG_ERR("The current configured Send Format does not support standard I/O output");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }

                if(recvformat != RECV_FORMAT_RB)
                {
                    RTMCLOG_ERR("Missing arguments related standard I/O input");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }

                DataFormatConverter_Ptr->Rrdrtpos2Uint8(jacmd,local_add_info,iocmd);

                res=SendProc_Common(REQ_EXEC_RTPOS,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) 
                {
                    DataFormatConverter_Ptr->Uint82Rrd_cur_pos(rrd_res.exData,pbufstatus,current_ja);
                    pcurja=current_ja;
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }

    short RTMCClient::Exec_RTMC(const vector<double>& jacmd,vector<double>& pcurja,unsigned int* pcurio,uint8_t* pbufstatus)
    {
        try{
        
        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
        
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(sizeof(double)*6+sizeof(unsigned short),0);
            short res=0;
            rrd_res.exData.assign(sizeof(uint8_t)+(sizeof(double)*6)+sizeof(unsigned int),0);

            if(DDSCommunication_Ptr!=nullptr)
            {

                if( sendformat != SEND_FORMAT_RB)
                {
                    RTMCLOG_ERR("Missing arguments related standard I/O output");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }

                if(recvformat != RECV_FORMAT_RBIO)
                {
                    RTMCLOG_ERR("The current configured Send Format does not support standard I/O input");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }

            
                DataFormatConverter_Ptr->Rrdrtpos2Uint8(jacmd,local_add_info);
        
                res=SendProc_Common(REQ_EXEC_RTPOS,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS)
                {
                    DataFormatConverter_Ptr->Uint82Rrd_cur_pos(rrd_res.exData,pbufstatus,current_ja,pcurio);
                    pcurja=current_ja;
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }

            timed_mtx_common_.unlock();
            return res;
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }

    short RTMCClient::Exec_RTMC(const vector<double>& jacmd,vector<double>& pcurja,unsigned short iocmd,unsigned int* pcurio,uint8_t* pbufstatus)
    {
        try{
        
        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
        
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(sizeof(double)*6+sizeof(unsigned short),0);
            short res=0;
            rrd_res.exData.assign(sizeof(uint8_t)+(sizeof(double)*6)+sizeof(unsigned int),0);

            if(DDSCommunication_Ptr!=nullptr)
            {

                if(sendformat != SEND_FORMAT_RBIO) 
                {
                    RTMCLOG_ERR("The current configured Send Format does not support standard I/O output");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }

                if(recvformat != RECV_FORMAT_RBIO) 
                {
                    RTMCLOG_ERR("The current configured Send Format does not support standard I/O input");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }



                DataFormatConverter_Ptr->Rrdrtpos2Uint8(jacmd,local_add_info,iocmd);

                res=SendProc_Common(REQ_EXEC_RTPOS,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS){
                    DataFormatConverter_Ptr->Uint82Rrd_cur_pos(rrd_res.exData,pbufstatus,current_ja,pcurio);
                    pcurja=current_ja;
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }

            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }

    short RTMCClient::Get_CurrentJA(vector<double>& pcur_ja)
    {
        try{
        
        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            RRDCoreData rrd_res={0,0,{}};
            rrd_res.exData.assign(sizeof(double)*6,0);
            vector<uint8_t> local_add_info;
            local_add_info.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
                short res=0;
                if(!DDSCommunication_Ptr->DDSConnection())
                {
                        DisconnectRC();
                        RTMCLOG_ERR("The Connection with the controller has been disconnected");
                        timed_mtx_common_.unlock();
                        return CLIENT_COM_ERR;
                }
                
                res=Update_OperationMode();
                if(res != CLIENT_SUCCESS)
                {
                    DisconnectRC();
                    timed_mtx_common_.unlock();
                    return res;
                }

                if(Check_OperationModeAuto()){

                    res=SendProc_Common(REQ_GET_CURPOS_INFO,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                    if(res==CLIENT_SUCCESS) 
                    {
                        for(unsigned short i=0; i<RTMC_MAX_JOINT; i++) DataFormatConverter_Ptr->Uint82Double(rrd_res.exData,&current_ja[i],i*8);
                        pcur_ja=current_ja;
                    }
                    if(res==CLIENT_ERR)
                    {
                        res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
                        if(res==CLIENT_OPERATION_MODE_ERR)
                        {
                            res=Update_CurJA();
                            if(res !=CLIENT_SUCCESS)
                            {
                                DisconnectRC();
                                timed_mtx_common_.unlock();
                                return res;
                            }
                            pcur_ja=current_ja;
                            res=CLIENT_SUCCESS;
                        }
                        
                        
                    }
                }
                else
                {
                    res=Update_CurJA();
                    if(res !=CLIENT_SUCCESS)
                    {
                        DisconnectRC();
                        timed_mtx_common_.unlock();
                        return res;
                    }
                    pcur_ja=current_ja;
                }
                
                timed_mtx_common_.unlock();
                return res;
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }

    short RTMCClient::Get_RCStatus(RCSTATUS& prcstatus)
    {
        try{
        
        if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;

            if(DDSCommunication_Ptr!=nullptr)
            {
                short res=0;

                if(!DDSCommunication_Ptr->DDSConnection())
                {
                        DisconnectRC();
                        RTMCLOG_ERR("The Connection with the controller has been disconnected");
                        timed_mtx_common_.unlock();
                        return CLIENT_COM_ERR;
                }

                res=Update_RCSts();
                if(res != CLIENT_SUCCESS)
                {
                    DisconnectRC();
                    timed_mtx_common_.unlock();
                    return res;
                }

                prcstatus.err_info.msg.clear();
                prcstatus.wrn_info.msg.clear();
                prcstatus=rc_status;

                timed_mtx_common_.unlock();
                return res;
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            
        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}
    }


    short RTMCClient::Reset()
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
                res=SendProc_Common(REQ_EXEC_RESET,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Set_BufferSize(unsigned short bufsize)
    {
        try{

            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
                if(bufsize == 2 ||
                bufsize == 5 ||         
                bufsize == 10)
                {
                    res=SendProc_Common(REQ_SET_BUF_SIZE,bufsize,RTMC_NONDATA,local_add_info,rrd_res);
                    if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
                }
                else
                {
                    RTMCLOG_ERR("The specified buffer size is not supported");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }           
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_BufferSize(unsigned short* bufsize)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);

            if(DDSCommunication_Ptr!=nullptr)
            {
            
                res=SendProc_Common(REQ_GET_BUF_SIZE,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS){
                    if(rrd_res.rrd_res_low == 2 ||
                    rrd_res.rrd_res_low == 5 ||
                    rrd_res.rrd_res_low == 10)
                    {
                        *bufsize=rrd_res.rrd_res_low;
                    }
                    else
                    {
                        DisconnectRC();
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_BufferSize",1018,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);   

            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Set_DigitalOutput_Bit(uint8_t bitnum, uint8_t bitdata)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);
            uint8_t FailedDetail=0;
            
            if(DDSCommunication_Ptr!=nullptr)
            {
                if(bitnum > STANDARD_DO_BIT_RANGE_MAX || ((bitdata != STANDARD_IO_HIGH) && (bitdata != STANDARD_IO_LOW)))
                {
                    RTMCLOG_ERR("The argument is out of range");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }
                memcpy(&local_add_info[0],&bitdata,sizeof(uint8_t));
                res=SendProc_Common(REQ_SET_DO,MAKEUSHORT(bitnum,RTMC_BITRANGE),RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR)
                {
                    res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low,&FailedDetail);
                    if(res==CLIENT_ERR)
                    {
                        if(FailedDetail == ASSIGEND_REMOTEIO)
                        {
                            RTMCLOG_ERR("Remote IO is assigned to standard IO output");
                        }else{
                            RTMCLOG_ERR_EX("Undefined error has occurred in Set_DigitalOutput_Bit",1019,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                            DisconnectRC();
                            timed_mtx_common_.unlock();
                            return CLIENT_UNDEF_ERR;
                        }
                    }
                }
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Set_DigitalOutput_Byte(uint8_t bytenum, uint8_t bytedata)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(1,0);
            uint8_t FailedDetail=0;
            
            if(DDSCommunication_Ptr!=nullptr)
            {
                if(bytenum > STANDARD_DO_BYTE_RANGE_MAX)
                {
                    RTMCLOG_ERR("The argument is out of range");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                } 
                memcpy(&local_add_info[0],&bytedata,sizeof(uint8_t));
                res=SendProc_Common(REQ_SET_DO,MAKEUSHORT(bytenum,RTMC_BYTERABGE),RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR)
                {
                    res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low,&FailedDetail);
                    if(res==CLIENT_ERR)
                    {
                        if(FailedDetail == ASSIGEND_REMOTEIO)
                        {
                            RTMCLOG_ERR("Remote IO is assigned to standard IO output");
                        }else{
                            RTMCLOG_ERR_EX("Undefined error has occurred in Set_DigitalOutput_Bit",1019,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                            DisconnectRC();
                            timed_mtx_common_.unlock();
                            return CLIENT_UNDEF_ERR;
                        }
                    }
                }
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Set_DigitalOutput_Word(unsigned short worddata)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            local_add_info.assign(sizeof(unsigned short),0);
            rrd_res.exData.assign(1,0);
            uint8_t FailedDetail=0;
            
            if(DDSCommunication_Ptr!=nullptr)
            {
            
                DataFormatConverter_Ptr->Ushort2Uint8(worddata,local_add_info,0);
                res=SendProc_Common(REQ_SET_DO,MAKEUSHORT(RTMC_NONDATA,RTMC_WORDRANGE),RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR)
                {
                    res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low,&FailedDetail);
                    if(res==CLIENT_ERR){
                        if(FailedDetail == ASSIGEND_REMOTEIO)
                        {
                            RTMCLOG_ERR("Remote IO is assigned to standard IO output");
                        }else{
                            RTMCLOG_ERR_EX("Undefined error has occurred in Set_DigitalOutput_Bit",1019,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                            DisconnectRC();
                            timed_mtx_common_.unlock();
                            return CLIENT_UNDEF_ERR;
                        }
                    }
                }
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_DigitalOutput_Bit(uint8_t bitnum, uint8_t* pbitdata)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            unsigned short temp_bitdata=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(sizeof(unsigned short),0);
            
            if(DDSCommunication_Ptr!=nullptr)
            {
                if(bitnum > STANDARD_DO_BIT_RANGE_MAX)
                {
                    RTMCLOG_ERR("The argument is out of range");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                } 
                res=SendProc_Common(REQ_GET_DO,MAKEUSHORT(bitnum,RTMC_BITRANGE),RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) 
                {
                DataFormatConverter_Ptr->Uint82Ushort(rrd_res.exData,&temp_bitdata,0);
                    if(temp_bitdata == STANDARD_IO_HIGH || temp_bitdata == STANDARD_IO_LOW )
                    {
                        *pbitdata=static_cast<uint8_t>(temp_bitdata);
                    }else{
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_DigitalOutput_Bit",1020,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        DisconnectRC();
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_DigitalOutput_Byte(uint8_t bytenum, uint8_t* pbytedata)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            unsigned short temp_bytedata=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(sizeof(unsigned short),0);
            
            if(DDSCommunication_Ptr!=nullptr)
            {
                if(bytenum > STANDARD_DO_BYTE_RANGE_MAX)
                {
                    RTMCLOG_ERR("The argument is out of range");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                } 
                res=SendProc_Common(REQ_GET_DO,MAKEUSHORT(bytenum,RTMC_BYTERABGE),RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) 
                {
                    DataFormatConverter_Ptr->Uint82Ushort(rrd_res.exData,&temp_bytedata,0);
                    if(temp_bytedata<=STANDARD_IO_BYTE_MAX)
                    {
                        *pbytedata=static_cast<uint8_t>(temp_bytedata);
                    }else{
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_DigitalOutput_Byte",1020,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        DisconnectRC();
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_DigitalOutput_Word(unsigned short* pworddata)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            unsigned short temp_worddata=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(sizeof(unsigned short),0);
            
            if(DDSCommunication_Ptr!=nullptr)
            {

                res=SendProc_Common(REQ_GET_DO,MAKEUSHORT(RTMC_NONDATA,RTMC_WORDRANGE),RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) 
                {
                DataFormatConverter_Ptr->Uint82Ushort(rrd_res.exData,&temp_worddata,0);
                *pworddata=temp_worddata;
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_DigitalInput_Bit(uint8_t bitnum, uint8_t* pbitdata)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            unsigned short temp_bitdata=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(sizeof(unsigned short),0);
            
            if(DDSCommunication_Ptr!=nullptr)
            {
                if(bitnum > STANDARD_DI_BIT_RANGE_MAX)
                {
                    RTMCLOG_ERR("The argument is out of range");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                } 
                res=SendProc_Common(REQ_GET_DI,MAKEUSHORT(bitnum,RTMC_BITRANGE),RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) 
                {
                    DataFormatConverter_Ptr->Uint82Ushort(rrd_res.exData,&temp_bitdata,0);
                    if(temp_bitdata == STANDARD_IO_HIGH || temp_bitdata == STANDARD_IO_LOW )
                    {
                        *pbitdata=static_cast<uint8_t>(temp_bitdata);
                    }else{
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_DigitalInput_Bit",1021,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        DisconnectRC();
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_DigitalInput_Byte(uint8_t bytenum, uint8_t* pbytedata)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            unsigned short temp_bytedata=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(sizeof(unsigned short),0);
            
            if(DDSCommunication_Ptr!=nullptr)
            {
                if(bytenum > STANDARD_DI_BYTE_RANGE_MAX)
                {
                    RTMCLOG_ERR("The argument is out of range");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }
                res=SendProc_Common(REQ_GET_DI,MAKEUSHORT(bytenum,RTMC_BYTERABGE),RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) 
                {
                    DataFormatConverter_Ptr->Uint82Ushort(rrd_res.exData,&temp_bytedata,0);
                    if(temp_bytedata<=STANDARD_IO_BYTE_MAX)
                    {
                        *pbytedata=static_cast<uint8_t>(temp_bytedata);
                    }else{
                        RTMCLOG_ERR_EX("Undefined error has occurred in Get_DigitalInput_Byte",1021,rrd_res.rrd_res_hi,rrd_res.rrd_res_low);
                        DisconnectRC();
                        timed_mtx_common_.unlock();
                        return CLIENT_UNDEF_ERR;
                    }
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_DigitalInput_Word(uint8_t wordnum, unsigned short* pworddata)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            short res=0;
            unsigned short temp_worddata=0;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(sizeof(unsigned short),0);
            
            if(DDSCommunication_Ptr!=nullptr)
            {
                if(wordnum > STANDARD_DI_WORD_RANGE_MAX)
                {
                    RTMCLOG_ERR("The argument is out of range");
                    timed_mtx_common_.unlock();
                    return CLIENT_ARG_ERR;
                }
                res=SendProc_Common(REQ_GET_DI,MAKEUSHORT(wordnum,RTMC_WORDRANGE),RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) 
                {
                DataFormatConverter_Ptr->Uint82Ushort(rrd_res.exData,&temp_worddata,0);
                *pworddata=temp_worddata;
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Set_Weight(double set_weight)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(sizeof(double),0);
            rrd_res.exData.assign(0,0);
            short res=0;
        
            if(DDSCommunication_Ptr!=nullptr)
            {

                if(RBWeightInertiaParam_Ptr->CanExc_LoadParam())
                {
                    if((set_weight<0.0) || (set_weight > RBWeightInertiaParam_Ptr->Load_MaxWeight()))
                    {
                        RTMCLOG_ERR("The argument is out of range");
                        timed_mtx_common_.unlock();
                        return CLIENT_ARG_ERR;  
                    }
                }else{
                    timed_mtx_common_.unlock();
                    return CLIENT_RBCHK_ERR; 
                }

                DataFormatConverter_Ptr->Double2Uint8(set_weight,local_add_info,0);

                rrd_res.rrd_res_low=0;
                rrd_res.rrd_res_hi=0;
                rrd_res.exData.clear();

                res=SendProc_Common(REQ_SET_WEIGHT,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_Weight(double* get_weight)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(sizeof(double),0);
            short res=0;
        
            if(DDSCommunication_Ptr!=nullptr)
            {
                res=SendProc_Common(REQ_GET_WEIGHT,RTMC_NONDATA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS) DataFormatConverter_Ptr->Uint82Double(rrd_res.exData,get_weight,0);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Set_Inertia(double set_inertia)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(sizeof(double),0);
            rrd_res.exData.assign(0,0);
            short res=0;
        
            if(DDSCommunication_Ptr!=nullptr)
            {

                if(RBWeightInertiaParam_Ptr->CanExc_LoadParam())
                {
                    if((set_inertia<0.0) || (set_inertia > RBWeightInertiaParam_Ptr->Load_MaxInertia()))
                    {
                        RTMCLOG_ERR("The argument is out of range");
                        timed_mtx_common_.unlock();
                        return CLIENT_ARG_ERR;  
                    }
                }else{
                    timed_mtx_common_.unlock();
                    return CLIENT_RBCHK_ERR; 
                }


                DataFormatConverter_Ptr->Double2Uint8(set_inertia,local_add_info,0);

                rrd_res.rrd_res_low=0;
                rrd_res.rrd_res_hi=0;
                rrd_res.exData.clear();

                res=SendProc_Common(REQ_SET_INERTIA,ONLY_INERTIA,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Set_Inertia(double set_inertia, unsigned short set_eccentricity)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(sizeof(double)+sizeof(unsigned short),0);
            rrd_res.exData.assign(0,0);
            short res=0;
        
            if(DDSCommunication_Ptr!=nullptr)
            {

                if(RBWeightInertiaParam_Ptr->CanExc_LoadParam())
                {
                    if((set_inertia<0.0) || (set_inertia > RBWeightInertiaParam_Ptr->Load_MaxInertia()))
                    {
                        RTMCLOG_ERR("The inertia parameter is out of range");
                        timed_mtx_common_.unlock();
                        return CLIENT_ARG_ERR;  
                    }

                    if(set_eccentricity > RBWeightInertiaParam_Ptr->Load_Eccentricity())
                    {
                        RTMCLOG_ERR("The eccentricity parameter is out of range");
                        timed_mtx_common_.unlock();
                        return CLIENT_ARG_ERR;  
                    }

                }else{
                    timed_mtx_common_.unlock();
                    return CLIENT_RBCHK_ERR; 
                }

                unsigned short index=0;
                DataFormatConverter_Ptr->Double2Uint8(set_inertia,local_add_info,index);
                index += sizeof(double);
                DataFormatConverter_Ptr->Ushort2Uint8(set_eccentricity,local_add_info,index);

                rrd_res.rrd_res_low=0;
                rrd_res.rrd_res_hi=0;
                rrd_res.exData.clear();

                res=SendProc_Common(REQ_SET_INERTIA,INERTIA_AND_ECC,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::Get_Inertia(double* get_inertia , unsigned short* get_eccentricity)
    {
        try{
        
            if(!timed_mtx_common_.try_lock_for(chrono::seconds(3))) throw true;
            
            RRDCoreData rrd_res={0,0,{}};
            vector<uint8_t> local_add_info;
            local_add_info.assign(1,0);
            rrd_res.exData.assign(sizeof(double)+sizeof(unsigned short),0);
            short res=0;
            unsigned short low_word_data= ONLY_INERTIA;   

            if(get_eccentricity != nullptr) low_word_data = INERTIA_AND_ECC;

            if(DDSCommunication_Ptr!=nullptr)
            {
                res=SendProc_Common(REQ_GET_INERTIA,low_word_data,RTMC_NONDATA,local_add_info,rrd_res);
                if(res==CLIENT_SUCCESS)
                {
                    unsigned short index=0;
                    DataFormatConverter_Ptr->Uint82Double(rrd_res.exData,get_inertia,index);
                    index += sizeof(double);
                    if(get_eccentricity != nullptr) DataFormatConverter_Ptr->Uint82Ushort(rrd_res.exData, get_eccentricity, index);
                }
                if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
            
            }else{
                RTMCLOG_ERR("The Connection with the controller is not established");
                timed_mtx_common_.unlock();
                return CLIENT_COM_ERR;
            }
            timed_mtx_common_.unlock();
            return res;

        }
        catch(bool timeout) {return CLIENT_TIMEOUT_ERR;}

    }

    short RTMCClient::SendProc_Common(unsigned short cmdID,unsigned short low_word,unsigned short hi_word, const vector<uint8_t>& add_data,RRDCoreData& rrd_res_core)
    {

        
            int ret=false;
            int res=0;
            RRD rrd_cmd={0,0,{}};
            RRD rrd_res={0,0,{}};
            rrd_cmd.exData.assign(1,0);
            rrd_res.exData.assign(1,0);
            

                if(!DDSCommunication_Ptr->DDSConnection())
                {
                    RTMCLOG_ERR("The Connection with the controller has been disconnected");
                    DisconnectRC();
                    return CLIENT_COM_ERR;
                }

                ret=DDSCommunication_Ptr->MakeRRDdata(cmdID,MAKEWORD(low_word,hi_word),add_data,rrd_cmd);
                if(!ret)
                {
                RTMCLOG_ERR("Undefined error has occurred in MakeRRDdata"); 
                DisconnectRC();
                return CLIENT_UNDEF_ERR;
                }
    
                res=DDSCommunication_Ptr->DDS_Send(rrd_cmd,&rrd_res);
                
                rrd_res_core.rrd_res_hi=HIWORD(rrd_res.dwordparam);
                rrd_res_core.rrd_res_low=LOWORD(rrd_res.dwordparam);
                rrd_res_core.exData=rrd_res.exData;
                
                if(res==TIMEOUT_ERR)
                {
                    DisconnectRC();
                    return CLIENT_COM_ERR;
                }else if(res == UNDEFINED_ERR)
                {
                    DisconnectRC();
                    return CLIENT_UNDEF_ERR;
                }else{
                    if(rrd_res_core.rrd_res_hi)
                    {
                        ret=DDSCommunication_Ptr->Check_exDataSize(cmdID, low_word, rrd_res_core);
                        if(!ret)
                        {
                            RTMCLOG_ERR("Invalid size of retrieved extra data");
                            DisconnectRC();
                            return CLIENT_UNDEF_ERR;
                        }
                    }

                    if(!rrd_res_core.rrd_res_hi) return CLIENT_ERR;
                }
            
            
            return CLIENT_SUCCESS;
        
    }

    short RTMCClient::Convert_RTMCRCErrInfo(unsigned short ErrInfo,uint8_t* plword_hbyte)
    {
        short err=0;
        switch(LOWBYTE(ErrInfo))
        {
            case RTMCRC_EXEC_ERR: 
            err=CLIENT_RCERR_STATUS_ERR;

            if(plword_hbyte != nullptr) 
            {
                *plword_hbyte=HIBYTE(ErrInfo);
                if(*plword_hbyte != 0) err=CLIENT_ERR;
            }
            break;
            
            case RTMCRC_OPERATION_MODE_ERR:  err=CLIENT_OPERATION_MODE_ERR; break;
            case RTMCRC_RBCHECK_ERR: err=CLIENT_RBCHK_ERR; break;
            case RTMCRC_RTMCMODE_ERR: err=CLIENT_RTMC_MODE_ERR; break;
            case RTMCRC_TIMEOUT_ERR: err=CLIENT_RCERR_STATUS_ERR; break;
            case RTMCRC_ERRSTATUS_ERR: err=CLIENT_RCERR_STATUS_ERR; break;
            case RTMCRC_ESTOPSTATUS_ERR: err=CLIENT_RCESTOP_STATUS_ERR; break;
            case RTMCRC_SGSTATUS_ERR: err=CLIENT_RCSG_STATUS_ERR; break;
            default: 
                RTMCLOG_ERR("Disconnecting from epson robot controller due to an undefined controller response");
                err=CLIENT_UNDEF_ERR;
                DisconnectRC();
                break;
        }

        return err;
    }

    short RTMCClient::Update_RCSts()
    {
        RRD rrd_rc_status={0,0,{}};
        bool RCStsUpdate=false;
        bool ret=false;
        short res=CLIENT_SUCCESS;
        INTERNALRCSTATUS temp_rc_status={0,0,0,0,0,{0,0,0},{0,0,0}};

        rrd_rc_status.exData.assign(INTERNAL_RC_STATUS_SIZE,0);
        
            ret=DDSCommunication_Ptr->Get_NewStatus(SYS_NOTICE_RC_STATUS,&rrd_rc_status,&RCStsUpdate);
            if(!ret) return CLIENT_UNDEF_ERR;
            if(RCStsUpdate) 
            {
                DataFormatConverter_Ptr->Uint82Rc_status(rrd_rc_status.exData,&temp_rc_status);
                res=Update_RCSts_Core(&rc_status,temp_rc_status);
                
            }
        return res;
    }

    short RTMCClient::Update_OperationMode()
    {
        RRD rrd_rc_status={0,0,{}};
        bool ret=false;
        bool RCStsUpdate=false;
        INTERNALRCSTATUS temp_rc_status={0,0,0,0,0,{0,0,0},{0,0,0}};
        short res=CLIENT_SUCCESS;
        
        ret=DDSCommunication_Ptr->Get_NewStatus(SYS_NOTICE_RC_STATUS,&rrd_rc_status,&RCStsUpdate);
        if(!ret) return CLIENT_UNDEF_ERR;
        if(RCStsUpdate){
            DataFormatConverter_Ptr->Uint82Rc_status(rrd_rc_status.exData,&temp_rc_status);
            res=Update_RCSts_Core(&rc_status,temp_rc_status);
        }   

        return res;

    }

    short RTMCClient::Update_RCSts_Core(RCSTATUS* prc_status, INTERNALRCSTATUS internal_rc_status)
    {
        bool update_errwrn=false;
        short res=CLIENT_SUCCESS;
        RRDCoreData rrd_res={0,0,{}};
        vector<uint8_t> local_add_info;
        rrdErrWrnMsgInfo temp_msginfo={"",""};
        local_add_info.assign(1,0);
        
        prc_status->safeGuard=internal_rc_status.safeGuard; 
        prc_status->eStop=internal_rc_status.eStop;
        prc_status->operation_mode=internal_rc_status.operation_mode;
    
        if(prc_status->err_num != internal_rc_status.err_num){
            prc_status->err_num=internal_rc_status.err_num;
            prc_status->err_info.add_info1=internal_rc_status.err_info.add_info1;
            prc_status->err_info.add_info2=internal_rc_status.err_info.add_info2;
            prc_status->err_info.jnt=internal_rc_status.err_info.jnt;
            update_errwrn=true;
        }

        if(prc_status->wrn_num != internal_rc_status.wrn_num){
            prc_status->wrn_num=internal_rc_status.wrn_num;
            prc_status->wrn_info.add_info1=internal_rc_status.wrn_info.add_info1;
            prc_status->wrn_info.add_info2=internal_rc_status.wrn_info.add_info2;
            prc_status->wrn_info.jnt=internal_rc_status.wrn_info.jnt;
            update_errwrn=true;
        }


        if(update_errwrn)
        {
            res=SendProc_Common(SYS_REQ_RC_ERRMSG,prc_status->err_num,prc_status->wrn_num,local_add_info,rrd_res);
            if(res==CLIENT_SUCCESS) 
            {
                DataFormatConverter_Ptr->Uint82Rrd_ErrWrnMsg_Info(rrd_res.exData,&temp_msginfo);
                prc_status->err_info.msg.clear();
                prc_status->wrn_info.msg.clear();
                prc_status->err_info.msg=temp_msginfo.err_msg;
                prc_status->wrn_info.msg=temp_msginfo.wrn_msg;
            }
            if(res==CLIENT_ERR) res=Convert_RTMCRCErrInfo(rrd_res.rrd_res_low);
        }
        
        return res;

    }

    short RTMCClient::Update_CurJA()
    {
        RRD rrd_ja_status={0,0,{}};
        bool JAStsUpdate=false;
        bool ret=false;

        rrd_ja_status.exData.assign(sizeof(double)*6,0);

        ret=DDSCommunication_Ptr->Get_NewStatus(SYS_NOTICE_CURPOS,&rrd_ja_status,&JAStsUpdate);
        if(!ret) return CLIENT_UNDEF_ERR;
        if(JAStsUpdate) for(unsigned short i=0; i<6; i++) DataFormatConverter_Ptr->Uint82Double(rrd_ja_status.exData,&current_ja[i],i*8);
        

        return CLIENT_SUCCESS;
    }

    bool RTMCClient::Check_OperationModeAuto()
    {
    if(rc_status.operation_mode == RC_OPERATION_MODE_Auto) return true;
    return false;
    }

    short RTMCClient::Check_ValidIP(const string& ip_address)
    {
        struct ifaddrs *ifaddress_lists, *ifaddress;
        char str_address[256];

        if(getifaddrs(&ifaddress_lists)==-1) 
        {
            RTMCLOG_ERR("Failed to check the available IP address on the Linux PC");
            return CLIENT_UNDEF_ERR;
        }

        for(ifaddress = ifaddress_lists; ifaddress != nullptr; ifaddress=ifaddress->ifa_next)
        {
            
            memset(str_address,0,sizeof(str_address));
            if(ifaddress->ifa_addr->sa_family == AF_INET){
                struct sockaddr_in *socket_address=(struct sockaddr_in *)ifaddress->ifa_addr;
                inet_ntop(AF_INET,&socket_address->sin_addr,str_address,sizeof(str_address));
                if(ip_address==str_address) return CLIENT_SUCCESS;
            }
        
        }

        RTMCLOG_ERR("The specified IP address is not available");
        return CLIENT_ARG_ERR;
    
    }

    short RTMCClient::Check_ValidPort(const string& ip_address, unsigned short port)
    {
    struct sockaddr_in temp_socket_address;
    int result=0;

        int temp_socket= socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if(temp_socket<0)
        {
            RTMCLOG_ERR_EX("Failed to check the available port number on the Linux PC","NONE",temp_socket,"NONE");
            return CLIENT_UNDEF_ERR;
        }
        temp_socket_address.sin_family=AF_INET;
        inet_pton(AF_INET, ip_address.c_str(),&temp_socket_address.sin_addr);
        temp_socket_address.sin_port=htons(port);

        result=bind(temp_socket,(struct sockaddr *)&temp_socket_address, sizeof(temp_socket_address));
        if(result != 0)
        {
            RTMCLOG_ERR("The specified port number is not available");

            return CLIENT_ARG_ERR;
        }
        close(temp_socket);
        return CLIENT_SUCCESS;
    
    }

    void RTMCClient::DisconnectRC()
    {
        delete DDSCommunication_Ptr;
        DDSCommunication_Ptr=nullptr;
    }

    bool RTMCClient::isIPAddress(const string& ip_address)
    {
        const regex ip_address_peattern(R"(^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]?|0)\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]?|0)$)");

        return regex_match(ip_address, ip_address_peattern);
    }
}
