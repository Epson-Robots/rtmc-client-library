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
#define CLIENT_SUCCESS      0
#define CLIENT_ERR          -1
#define CLIENT_OPERATION_MODE_ERR     -2
#define CLIENT_RBCHK_ERR    -3
#define CLIENT_RTMC_MODE_ERR     -4
#define CLIENT_RCERR_STATUS_ERR     -5
#define CLIENT_RCESTOP_STATUS_ERR     -6
#define CLIENT_RCSG_STATUS_ERR     -7
#define CLIENT_COM_ERR   -8
#define CLIENT_TIMEOUT_ERR  -9
#define CLIENT_ARG_ERR      -10
#define CLIENT_UNDEF_ERR    -11

#define RC_OPERATION_MODE_Program 0
#define RC_OPERATION_MODE_Auto    1
#define RC_OPERATION_MODE_Teach   2

#define RC_SG_OPEN 1
#define RC_SG_CLOSE 0

#define RC_ESTOP_ON 1
#define RC_ESTOP_OFF 0

#define RC_NONERR 0
#define RC_NONWARNNING 0

#define RTMC_SEND_FORMAT_RB 0
#define RTMC_SEND_FORMAT_RB_DO 1
#define RTMC_RECV_FORMAT_RB 0
#define RTMC_RECV_FORMAT_RB_DI 1

#define RTMC_BUFF_FULL 1
#define RTMC_BUFF_OK 0