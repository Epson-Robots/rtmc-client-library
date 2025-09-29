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
#include <string.h>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <unistd.h>

using namespace std;

class Create_Ref
{
    private:
    unsigned short io_update_period;
    double amplitude;
    unsigned short period;

    public:
    Create_Ref(double set_amplitude, unsigned short set_period, unsigned short set_io_update_period=0);
    ~Create_Ref();
    void Create_JARef(vector<double>& prtmc_ja_ref, int cnt);
    void Create_DORef(unsigned short* pdo, int cnt);
};