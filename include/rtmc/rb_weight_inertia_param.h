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
#include <map>
#include <vector>
#include <algorithm>

using namespace std;
namespace epson_rtmc_client
{
    class RBWeightInertiaParam{
    public: 
        RBWeightInertiaParam();
        ~RBWeightInertiaParam();

        private:
        string rb_model;

        vector<string> rb_model_list =
        {
        //GX4
        "GX4-C251S",
        "GX4-C251C",
        "GX4-C301S",
        "GX4-C301SM",
        "GX4-C301C",
        "GX4-C301CM",
        "GX4-C351S",
        "GX4-C351SM",
        "GX4-C351C",
        "GX4-C351CM",
        "GX4-C351S-L",
        "GX4-C351C-L",
        "GX4-C351S-R",
        "GX4-C351C-R",
        //GX8
        "GX8-C452S",
        "GX8-C452C",
        "GX8-C452SR",
        "GX8-C452CR",
        "GX8-C453S",
        "GX8-C453C",
        "GX8-C453SR",
        "GX8-C453CR",
        "GX8-C552S",
        "GX8-C552C",
        "GX8-C552SR",
        "GX8-C552CR",
        "GX8-C553S",
        "GX8-C553C",
        "GX8-C553SR",
        "GX8-C553CR",
        "GX8-C652S",
        "GX8-C652C",
        "GX8-C652SR",
        "GX8-C652CR",
        "GX8-C653S",
        "GX8-C653C",
        "GX8-C653SR",
        "GX8-C653CR",

        //RS4
        "RS4-C351S",
        "RS4-C351C",
        //RS6
        "RS6-C552S",
        "RS6-C552C",
        //C8
        "C8-C901S",
        "C8-C901C",
        "C8-C901SR",
        "C8-C901CR",
        "C8-C1401S",
        "C8-C1401C",
        "C8-C1401SR",
        "C8-C1401CR",
        //C12
        "C12-C1401S",
        "C12-C1401C",
        //CX4
        "CX4-A601S",
        "CX4-A601C",
        "CX4-A601SR",
        "CX4-A601CR",
        //CX7
        "CX7-A701S",
        "CX7-A701C",
        "CX7-A701SR",
        "CX7-A701CR",
        "CX7-A901S",
        "CX7-A901C",
        "CX7-A901SR",
        "CX7-A901CR"
        };


        map<string,double> RB_Maxweight_List=
        {
            //GX4
            {"GX4",4.0}, 
            //GX8
            {"GX8",8.0},
            //RS4
            {"RS4",4.0},
            //RS6
            {"RS6",6.0},
            //C8
            {"C8",8.0},
            //C12
            {"C12",12.0},
            //CX4
            {"CX4",5.0},
            //CX7
            {"CX7",7.0}
        };

        map<string,double> RB_Maxinertia_List=
        {
            //GX4
            {"GX4",0.05},
            //GX8
            {"GX8",0.16},
            //RS4
            {"RS4",0.05},
            //RS6
            {"RS6",0.12},
            //C8
            {"C8",0.15},
            //C12
            {"C12",0.20},
            //CX4
            {"CX4",0.1},
            //CX7
            {"CX7",0.15}

        };

        map<string,unsigned short> RB_Eccentricity_List=
        {
            //GX4
            {"GX4",150},
            //GX8
            {"GX8",150},
            //RS4
            {"RS4",100},
            //RS6
            {"RS6",100},
            //C8
            {"C8",300},
            //C12
            {"C12",300},
            //CX4
            {"CX4",200},
            //CX7
            {"CX7",300},
        };


        string Get_SpecificRBModelName(const string& rb_model);

        public:
        bool Set_RBModel(string set_rb_model);
        double Load_MaxWeight() {return RB_Maxweight_List[Get_SpecificRBModelName(rb_model)];};
        double Load_MaxInertia(){return RB_Maxinertia_List[Get_SpecificRBModelName(rb_model)];};
        unsigned short Load_Eccentricity(){return RB_Eccentricity_List[Get_SpecificRBModelName(rb_model)];};
        bool CanExc_LoadParam();
        void Reset_RBModel(){rb_model.clear();};
        
    };
}