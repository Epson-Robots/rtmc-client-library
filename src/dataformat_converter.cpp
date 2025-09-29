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

#include "rtmc/dataformat_converter.h"
#include <cstring>


namespace epson_rtmc_client
{
    DataFormatConverter::DataFormatConverter()
    {
        LittleEndian=Check_LittleEndian();
    }

    DataFormatConverter::~DataFormatConverter()
    {
        ;
    }

    bool DataFormatConverter::Check_LittleEndian()
    {
        unsigned short  us_temp=1;
        if(*(uint8_t *)&us_temp) return true;
        
        return false;

    }

    void DataFormatConverter::String2Uint8(string characer, vector<uint8_t>& pexdata,unsigned short index)
    { 
        memcpy(&pexdata[index],&characer[0],characer.size());
    }

    void DataFormatConverter::Double2Uint8(double ddata, vector<uint8_t>& pexdata,unsigned short index)
    {

        vector<uint8_t> temp_exdata;
        temp_exdata.assign(sizeof(double),0.0);
        memcpy(&temp_exdata[0],&ddata,sizeof(double));
        
        if(!LittleEndian)
        {
            ChangeEndian_B2L(temp_exdata,pexdata,sizeof(double),index);
        }else{
            memcpy(&pexdata[index],&temp_exdata[0],sizeof(double));
        }

    }

    void DataFormatConverter::Ushort2Uint8(unsigned short usdata, vector<uint8_t>& pexdata,unsigned short index)
    {

        vector<uint8_t> temp_exdata;
        temp_exdata.assign(sizeof(unsigned short),0);
        memcpy(&temp_exdata[0],&usdata,sizeof(double));
        
        if(!LittleEndian)
        {
            ChangeEndian_B2L(temp_exdata,pexdata,sizeof(unsigned short),index);
        }else{
            memcpy(&pexdata[index],&temp_exdata[0],sizeof(unsigned short));
        }
    }

    void DataFormatConverter::Rrdrtpos2Uint8(vector<double> rtpos, vector<uint8_t>& pexdata, unsigned short digitalOut)
    {
        for(unsigned short i=0; i<6;i++) Double2Uint8(rtpos[i],pexdata,i*8);
        Ushort2Uint8(digitalOut,pexdata,OFFSET_RRD_RT_POS_IO);
    }

    void DataFormatConverter::Uint82Double(vector<uint8_t> pexdata, double* ddata, unsigned short index)
    {
    
        if(!LittleEndian) {
            vector<uint8_t> temp_exdata;
            temp_exdata.assign(sizeof(double),0);
            
            ChangeEndian_L2B(pexdata,temp_exdata,sizeof(double),index);
            memcpy(ddata,&temp_exdata[0],sizeof(double));

        }else{
            
            memcpy(ddata,&pexdata[index],sizeof(double));
            
        }
    
    }

    void DataFormatConverter::Uint82Ushort(vector<uint8_t> pexdata, unsigned short* sdata, unsigned short index)
    {
        if(!LittleEndian) {
            vector<uint8_t> temp_exdata;
            temp_exdata.assign(sizeof(unsigned short),0);
            
            ChangeEndian_L2B(pexdata,temp_exdata,sizeof(unsigned short),index);
            memcpy(sdata,&temp_exdata[0],sizeof(unsigned short));

        }else{
            
            memcpy(sdata,&pexdata[index],sizeof(unsigned short));
        }
        
    }

    void DataFormatConverter::Uint82Uint(vector<uint8_t> pexdata, unsigned int* idata, unsigned short index)
    {
        if(!LittleEndian) {
            vector<uint8_t> temp_exdata;
            temp_exdata.assign(sizeof(unsigned int),0);
            
            ChangeEndian_L2B(pexdata,temp_exdata,sizeof(unsigned int),index);
            memcpy(idata,&temp_exdata[0],sizeof(unsigned int));

        }else{
            
            memcpy(idata,&pexdata[index],sizeof(unsigned int));
        }
        
    }

    void DataFormatConverter::Uint82Int(vector<uint8_t> pexdata, int* idata, unsigned short index)
    {
        if(!LittleEndian) {
            vector<uint8_t> temp_exdata;
            temp_exdata.assign(sizeof(int),0);
            
            ChangeEndian_L2B(pexdata,temp_exdata,sizeof(int),index);
            memcpy(idata,&temp_exdata[0],sizeof(int));

        }else{
            
            memcpy(idata,&pexdata[index],sizeof(int));
        }
        
    }

    void DataFormatConverter::Uint82String(vector<uint8_t> pexdata, string* str_data, unsigned short index,unsigned short msg_size)
    {
        string temp_str(reinterpret_cast<const char*>(&pexdata[index]),msg_size);
        *str_data=temp_str;
    }

    void DataFormatConverter::Uint82Rrd_cur_pos(vector<uint8_t> pexdata, uint8_t* pbufstatus, vector<double>& prrd_curpos, unsigned int* pdigitalIn)
    {
   
        if(pbufstatus != nullptr) memcpy(pbufstatus,&pexdata[0],sizeof(uint8_t));
            
        for(unsigned short i=0; i<6; i++) Uint82Double(pexdata,&prrd_curpos[i],static_cast<unsigned short>(OFFSET_RRD_CUR_POS_RB+i*8));

        if(pdigitalIn!=nullptr) Uint82Uint(pexdata,pdigitalIn,OFFSET_RRD_CUR_POS_IO);
            
    }

    void DataFormatConverter::Uint82Rc_status(vector<uint8_t> pexdata, INTERNALRCSTATUS* pinternal_rc_status)
    {
        uint8_t temp_estop_sg[2]={0,0};
        unsigned short temp_opmode_err_wrn[3]={0,0,0};
        int temp_add_info_err[3]={0,0,0};
        int temp_add_info_wrn[3]={0,0,0};
        unsigned short index=0;
    
        for(unsigned short i=0; i<2;i++) 
        {
            memcpy(&(temp_estop_sg[i]),&pexdata[i],sizeof(uint8_t));
            index++;
        }
        
        for(unsigned short i=0; i<3; i++)
        {
            Uint82Ushort(pexdata,&(temp_opmode_err_wrn[i]),index);
            index += sizeof(unsigned short);
        }

        
        for(unsigned short i=0; i<3; i++) 
        {
            if(pinternal_rc_status->err_num != temp_opmode_err_wrn[1]) Uint82Int(pexdata,&(temp_add_info_err[i]),index);
            index += sizeof(int);
        }

        for(unsigned short i=0; i<3; i++) 
        {
            if(pinternal_rc_status->err_num != temp_opmode_err_wrn[2]) Uint82Int(pexdata,&(temp_add_info_wrn[i]),index);
            index += sizeof(int);
        }

        pinternal_rc_status->safeGuard=temp_estop_sg[0];
        pinternal_rc_status->eStop=temp_estop_sg[1];
        pinternal_rc_status->operation_mode=temp_opmode_err_wrn[0];

        pinternal_rc_status->err_num=temp_opmode_err_wrn[1];
        pinternal_rc_status->err_info.add_info1=temp_add_info_err[0];
        pinternal_rc_status->err_info.add_info2=temp_add_info_err[1];
        pinternal_rc_status->err_info.jnt=static_cast<unsigned short>(temp_add_info_err[2]);
        
        pinternal_rc_status->wrn_num=temp_opmode_err_wrn[2];
        pinternal_rc_status->wrn_info.add_info1=temp_add_info_wrn[0];
        pinternal_rc_status->wrn_info.add_info2=temp_add_info_wrn[1];
        pinternal_rc_status->wrn_info.jnt=static_cast<unsigned short>(temp_add_info_wrn[2]);   
    }

    void DataFormatConverter::Uint82Rrd_ErrWrnMsg_Info(const vector<uint8_t>& pexdata, rrdErrWrnMsgInfo* msginfo)
    {
        unsigned short wrn_msg_size=0;
        unsigned short err_msg_size=0;
        unsigned short index=0;

        Uint82Ushort(pexdata,&wrn_msg_size,index);
        index += sizeof(unsigned short);

        if(wrn_msg_size>0)
        {
            msginfo->wrn_msg.resize(wrn_msg_size-1);
            Uint82String(pexdata,&(msginfo->wrn_msg),index,(wrn_msg_size-1));
            index += wrn_msg_size;
        }
    
        Uint82Ushort(pexdata,&err_msg_size,index);
        index += sizeof(unsigned short);

        if(err_msg_size>0)
        {
            msginfo->err_msg.resize(err_msg_size-1);
            Uint82String(pexdata,&(msginfo->err_msg),index,(err_msg_size-1));
        }

    }

    void DataFormatConverter::ChangeEndian_B2L(vector<uint8_t> in_exdata,vector<uint8_t>& out_exdata,unsigned short size, unsigned short index)
    {
        for(unsigned short i=0; i<size; i++) out_exdata[index+i]=in_exdata[static_cast<unsigned long>((size-1)-i)];
    }

    void DataFormatConverter::ChangeEndian_L2B(vector<uint8_t> in_exdata,vector<uint8_t>& out_exdata,unsigned short size, unsigned short index)
    {
        for(unsigned short i=0; i<size; i++) out_exdata[static_cast<unsigned long>((size-1)-i)]=in_exdata[index+i];
    }
}