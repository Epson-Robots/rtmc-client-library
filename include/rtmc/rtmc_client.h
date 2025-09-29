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
#include <string>

#include <mutex>
#include <memory>
#include <vector>

using namespace std;

namespace epson_rtmc_client
{

    class DDSCommunication;
    class RBWeightInertiaParam;
    class DataFormatConverter;
    struct RRD;
    struct RRDCoreData;
    struct INTERNALRCSTATUS;


    struct COMPROPERTY{
        string rc_ip_address;
        unsigned short rc_builtinmsg_port;
    };

    struct COMPROPERTYOPT{
        string client_ip_address;
        unsigned short client_builtinmsg_port;
        unsigned short client_userdata_port;
    };

    struct SECPROPERTY{
        bool security_communication_flag;
        string ca_cert_filepath;
        string client_cert_filepath;
        string client_private_key;
        string client_governance_filepath;
        string client_permissions_filepath;
    };

    struct RCErrWrnInfo{
        int add_info1;
        int add_info2;
        unsigned short jnt;
        string         msg;
    };

    struct RCSTATUS{
        uint8_t        safeGuard;
        uint8_t        eStop;
        unsigned short operation_mode;
        unsigned short err_num;
        unsigned short wrn_num;
        RCErrWrnInfo     err_info;
        RCErrWrnInfo     wrn_info;
    };

    class RTMCClient
    {
    public:
    RTMCClient();
    ~RTMCClient();
    RTMCClient(const RTMCClient&) = delete;
    RTMCClient& operator=(const RTMCClient&) = delete;

    short Set_ComProperty(const COMPROPERTY& set_com_property,COMPROPERTYOPT set_com_property_option={"",0,0});
    short Get_ComProperty(COMPROPERTY* pget_com_property, COMPROPERTYOPT* pget_com_property_option=nullptr);
    short Set_SecProperty(const SECPROPERTY& set_sec_property);
    short Get_SecProperty(SECPROPERTY* pget_sec_property);
    short RTMC_Connect(const string& password);
    short RTMC_DisConnect();
    short RBCheck(const string& rb_model);
    short RBCheckResult();
    short Get_RBModel(string* rb_model);
    short Set_RTMCSendFormat(unsigned short sendformat);
    short Get_RTMCSendFormat(unsigned short* sendformat);
    short Set_RTMCRecvFormat(unsigned short recvformat);
    short Get_RTMCRecvFormat(unsigned short* recvformat);
    short MotorOn();
    short MotorOff();
    short Get_MotorStatus(bool *pmotorstatus);
    short PowerHigh();
    short PowerLow();
    short Get_PowerMode(bool *ppwoermode);
    short Set_RTMCModeEnable();
    short Set_RTMCModeDisable();
    short Get_RTMCMode(bool *prtmcmode);
    short Exec_RTMC(const vector<double>& jacmd,vector<double>& pcurja,uint8_t* bufstatus=nullptr);
    short Exec_RTMC(const vector<double>& jacmd,vector<double>& pcurja, unsigned short iocmd,uint8_t* bufstatus=nullptr);
    short Exec_RTMC(const vector<double>& jacmd,vector<double>& pcurja, unsigned int* pcurio,uint8_t* bufstatus=nullptr);
    short Exec_RTMC(const vector<double>& jacmd,vector<double>& pcurja, unsigned short iocmd, unsigned int* pcurio,uint8_t* pbufstatus=nullptr);
    short Get_CurrentJA(vector<double>& pcurja);
    short Get_RCStatus(RCSTATUS& prcstatus);
    short Reset();
    short Set_BufferSize(unsigned short bufsize);
    short Get_BufferSize(unsigned short* bufsize);
    short Set_DigitalOutput_Bit(uint8_t bitnum,uint8_t bitdata);
    short Set_DigitalOutput_Byte(uint8_t bytenum,uint8_t bytedata);
    short Set_DigitalOutput_Word(unsigned short worddata);
    short Get_DigitalOutput_Bit(uint8_t bitnum,uint8_t* pbitdata);
    short Get_DigitalOutput_Byte(uint8_t bytenum,uint8_t* pbytedata);
    short Get_DigitalOutput_Word(unsigned short* pworddata);
    short Get_DigitalInput_Bit(uint8_t bitnum,uint8_t* pbitdata);
    short Get_DigitalInput_Byte(uint8_t bytenum,uint8_t* pbytedata);
    short Get_DigitalInput_Word(uint8_t wordnum,unsigned short* pworddata);
    short Set_Weight(double set_weight);
    short Get_Weight(double* get_weight);
    short Set_Inertia(double set_inertia);
    short Set_Inertia(double set_inertia , unsigned short set_eccentricity);
    short Get_Inertia(double* get_inertia , unsigned short* get_eccentricity=nullptr);

    private:
    short SendProc_Common(unsigned short cmdID,unsigned short low_word,unsigned short high_word, const vector<uint8_t>& add_data,RRDCoreData& rrd_res);
    short Convert_RTMCRCErrInfo(unsigned short ErrInfo,uint8_t* plword_hbyte=nullptr);
    short Update_RCSts();
    short Update_RCSts_Core(RCSTATUS* prc_status, INTERNALRCSTATUS internal_rc_status);
    short  Update_OperationMode();
    short Update_CurJA();
    bool Check_OperationModeAuto();
    short Check_ValidIP(const string& ip_address);
    short Check_ValidPort(const string& ip_address,unsigned short);
    bool isIPAddress(const string& ip_address);
    void DisconnectRC();

    COMPROPERTY com_property;
    COMPROPERTYOPT com_property_option;
    bool com_property_option_flag;
    SECPROPERTY sec_property;
    vector<double> current_ja;
    RCSTATUS rc_status;
    unsigned short sendformat;
    unsigned short recvformat;

    timed_mutex timed_mtx_common_;

    DDSCommunication*  DDSCommunication_Ptr;
    RBWeightInertiaParam* RBWeightInertiaParam_Ptr;
    DataFormatConverter* DataFormatConverter_Ptr;

    };
}