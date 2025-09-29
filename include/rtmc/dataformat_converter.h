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
#include <vector>
#include <string>

using namespace std;


#define OFFSET_STRDATA                      2
#define OFFSET_RRD_CUR_POS_RB               8
#define OFFSET_RRD_CUR_POS_IO               56
#define OFFSET_RRD_RT_POS_IO                48

namespace epson_rtmc_client
{

    struct InternalRCErrWrnInfo{
        int add_info1;
        int add_info2;
        unsigned short jnt;
    };

    struct INTERNALRCSTATUS{
        uint8_t        safeGuard;
        uint8_t        eStop;
        unsigned short operation_mode;
        unsigned short err_num;
        unsigned short wrn_num;
        InternalRCErrWrnInfo     err_info;
        InternalRCErrWrnInfo     wrn_info;
    };

    struct rrdErrWrnMsgInfo{   
        string wrn_msg;
        string err_msg; 
    };

    class DataFormatConverter
    {
        public:
        DataFormatConverter();
        ~DataFormatConverter();

        void String2Uint8(string character, vector<uint8_t>& pexdata,unsigned short index);
        void Double2Uint8(double ddata, vector<uint8_t>& pexdata, unsigned short index);
        void Ushort2Uint8(unsigned short usdata, vector<uint8_t>& pexdata, unsigned short index);

        void Rrdrtpos2Uint8(vector<double> rtpos, vector<uint8_t>& pexdata, unsigned short digitalOut=0);
        
        void Uint82Double(vector<uint8_t> pexdata, double* ddata,unsigned short index);
        void Uint82Ushort(vector<uint8_t> pexdata, unsigned short* sdata, unsigned short index);
        void Uint82Uint(vector<uint8_t> pexdata, unsigned int* idata, unsigned short index);
        void Uint82String(vector<uint8_t> pexdata, string* str_data, unsigned short index, unsigned short msg_size);
        void Uint82Int(vector<uint8_t> pexdata, int* idata, unsigned short index);

        void Uint82Rc_status(vector<uint8_t> pexdata, INTERNALRCSTATUS* pinternal_rc_status);
        void Uint82Rrd_cur_pos(vector<uint8_t> pexdata, uint8_t* pbufstatus, vector<double>& prrd_curpos, unsigned int* pdigitalIn=nullptr);
        void Uint82Rrd_ErrWrnMsg_Info(const vector<uint8_t>& pexdata, rrdErrWrnMsgInfo* pmsginfo);

        private:
        bool LittleEndian=true;

        bool Check_LittleEndian();
        void ChangeEndian_B2L(vector<uint8_t> exdata,vector<uint8_t>& pexdata,unsigned short size, unsigned short index);
        void ChangeEndian_L2B(vector<uint8_t> exdata,vector<uint8_t>& pexdata,unsigned short size, unsigned short index);

    };
}