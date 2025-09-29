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
#include "epson_rtmc_demo.h"

Create_Ref::Create_Ref(double set_amplitude, unsigned short set_period, unsigned short set_io_update_period)
{
    io_update_period=set_io_update_period;
    amplitude=set_amplitude;
    period=set_period;
}

Create_Ref::~Create_Ref()
{
    ;
}

void Create_Ref::Create_JARef(vector<double>& p_ja_ref, int cnt)
{
  if(cnt <= period)
  {
    double ja_val=0.0;
    ja_val=amplitude*sin(2*M_PI*cnt/period);
    p_ja_ref[0] += ja_val;
  }
}

void Create_Ref::Create_DORef(unsigned short* pdo, int cnt)
{
  if(cnt <= period)
  {
    if(cnt % io_update_period == 0) *pdo=~(*pdo);
  }
}
