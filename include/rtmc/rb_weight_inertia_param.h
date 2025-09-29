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
        "GX4-C251S",
        "GX4-C301S",
        "GX4-C351S",
        "GX4-C351S-L",
        "GX4-C351S-R",
        "C8-C901S",
        "C8-C1401S",
        "C12-C1401S"
        };


        map<string,double> RB_Maxweight_List=
        {
            {rb_model_list[0],4.0}, //GX4-C251S
            {rb_model_list[1],4.0}, //GX4-C301S
            {rb_model_list[2],4.0}, //GX4-C351S
            {rb_model_list[3],4.0}, //GX4-C351S-L
            {rb_model_list[4],4.0}, //GX4-C351S-R
            {rb_model_list[5],8.0}, //C8-C901S
            {rb_model_list[6],8.0}, //C8-C1401S
            {rb_model_list[7],12.0} //C12-C1401S

        };

        map<string,double> RB_Maxinertia_List=
        {
            {rb_model_list[0],0.05}, //GX4-C251S
            {rb_model_list[1],0.05}, //GX4-C301S
            {rb_model_list[2],0.05}, //GX4-C351S
            {rb_model_list[3],0.05}, //GX4-C351S-L
            {rb_model_list[4],0.05}, //GX4-C351S-R
            {rb_model_list[5],0.15}, //C8-C901S
            {rb_model_list[6],0.15}, //C8-C1401S
            {rb_model_list[7],0.20}  //C12-C1401S

        };

        map<string,unsigned short> RB_Eccentricity_List=
        {
            {rb_model_list[0],150}, //GX4-C251S
            {rb_model_list[1],150}, //GX4-C301S
            {rb_model_list[2],150}, //GX4-C351S
            {rb_model_list[3],150}, //GX4-C351S-L
            {rb_model_list[4],150}, //GX4-C351S-R
            {rb_model_list[5],300}, //C8-C901S
            {rb_model_list[6],300}, //C8-C1401S
            {rb_model_list[7],300}  //C12-C1401S

        };




        public:
        bool Set_RBModel(string set_rb_model);
        double Load_MaxWeight() {return RB_Maxweight_List[rb_model];};
        double Load_MaxInertia(){return RB_Maxinertia_List[rb_model];};
        unsigned short Load_Eccentricity(){return RB_Eccentricity_List[rb_model];};
        bool CanExc_LoadParam();
        void Reset_RBModel(){rb_model.clear();};
        
    };
}