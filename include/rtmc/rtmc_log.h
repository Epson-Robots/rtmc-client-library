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
#include <iostream>

#define RTMC_RED_LOG "\33[31m"
#define RTMC_RESET_LOG "\33[m"
#define RTMC_ERR_PREFIX "[RTMC CLIENT LIB ERROR]"

#define RTMCLOG_ERR(msg) std::cout<<RTMC_RED_LOG<<RTMC_ERR_PREFIX<<msg<<RTMC_RESET_LOG<<std::endl
#define RTMCLOG_ERR_EX(msg,ID,high,low) std::cout<<RTMC_RED_LOG<<RTMC_ERR_PREFIX<<msg<<","<<ID<<","<<high<<","<<low<<RTMC_RESET_LOG<<std::endl
