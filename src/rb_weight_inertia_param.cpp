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

#include <sstream>

#include "rtmc/rb_weight_inertia_param.h"

namespace epson_rtmc_client
{
    RBWeightInertiaParam::RBWeightInertiaParam() : rb_model("")
    {
        ;
    }

    RBWeightInertiaParam::~RBWeightInertiaParam()
    {
        ;
    }

    bool RBWeightInertiaParam::Set_RBModel(string set_rb_model)
    {
        vector<string>::iterator temp_rb_model;
        temp_rb_model=find(rb_model_list.begin(),rb_model_list.end(),set_rb_model);
        if(temp_rb_model==rb_model_list.end()) return false;

        rb_model=set_rb_model;

        return true;
    }

    bool RBWeightInertiaParam::CanExc_LoadParam()
    {
        vector<string>::iterator temp_rb_model;
        temp_rb_model=find(rb_model_list.begin(),rb_model_list.end(),rb_model);
        if(temp_rb_model==rb_model_list.end()) return false;

        return true;

    }

    string RBWeightInertiaParam::Get_SpecificRBModelName(const string& rb_model)
    {
        stringstream ss_rb_model(rb_model);
        string  specific_rbmodel_name="";
        getline(ss_rb_model,specific_rbmodel_name,'-');
        return specific_rbmodel_name;
    }
}
