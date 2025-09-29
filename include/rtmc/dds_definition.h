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

#include <map>
#include <string>

#define SYS_NOTICE_RC_STATUS           0
#define SYS_NOTICE_CURPOS                   1

#define SYS_REQ_RC_STATUS                   100
#define SYS_REQ_CURPOS_INFO                 101
#define SYS_REQ_RTMC_FORMAT_INFO            102
#define SYS_REQ_RC_ERRMSG                   103

#define REQ_CONNECT                         1000
#define REQ_CHECK_ROBOTMODEL                1001
#define REQ_GET_ROBOTMODEL_CHECKRESULT      1002
#define REQ_GET_ROBOTMODEL                  1003
#define REQ_SET_RTPOS_IO_FORM               1004
#define REQ_GET_RTPOS_IO_FORM               1005
#define REQ_SET_CURPOS_IO_FORM              1006
#define REQ_GET_CURPOS_IO_FORM              1007
#define REQ_SET_MOTOR                       1008
#define REQ_GET_MOTOR                       1009
#define REQ_SET_POWER                       1010
#define REQ_GET_POWER                       1011
#define REQ_SET_ROBOT_CONTROL_MODE          1012
#define REQ_GET_ROBOT_CONTROL_MODE          1013
#define REQ_EXEC_RTPOS                      1014
#define REQ_GET_CURPOS_INFO                 1015
#define REQ_EXEC_RESET                      1016
#define REQ_SET_BUF_SIZE                    1017
#define REQ_GET_BUF_SIZE                    1018
#define REQ_SET_DO                          1019
#define REQ_GET_DO                          1020
#define REQ_GET_DI                          1021
#define REQ_SET_WEIGHT                      1022
#define REQ_GET_WEIGHT                      1023
#define REQ_SET_INERTIA                     1024
#define REQ_GET_INERTIA                     1025


#define REQ_CMDID_MIN                       1000
#define REQ_CMDID_MAX                       1025
#define SYS_NOTICE_CMDID_MIN                0
#define SYS_NOTICE_CMDID_MAX                1
#define SYS_REQ_CMDID_MIN                   100
#define SYS_REQ_CMDID_MAX                   103

#define SEND_SUCCESS                        0
#define TIMEOUT_ERR                         -1
#define UNDEFINED_ERR                       -2

#define INTERNAL_RC_STATUS_SIZE             22

#define RTMCRC_EXEC_ERR                     0
#define RTMCRC_OPERATION_MODE_ERR           1
#define RTMCRC_RBCHECK_ERR                  2
#define RTMCRC_RTMCMODE_ERR                 3
#define RTMCRC_TIMEOUT_ERR                  4
#define RTMCRC_ERRSTATUS_ERR                5
#define RTMCRC_ESTOPSTATUS_ERR              6
#define RTMCRC_SGSTATUS_ERR                 7

#define RTMC_MAX_JOINT                      6

#define RTMC_CONNECTION 1
#define RTMC_DISCONNECTION 0
#define RTMC_NONDATA 0
#define RTMC_BITRANGE 0
#define RTMC_BYTERABGE 1
#define RTMC_WORDRANGE 2
#define SEND_FORMAT_RB 0
#define SEND_FORMAT_RBIO 1
#define RECV_FORMAT_RB 0
#define RECV_FORMAT_RBIO 1
#define RTMC_MOTORON 1
#define RTMC_MOTOROFF 0
#define RTMC_POWERHIGH 1
#define RTMC_POWERLOW 0
#define RTMC_MODE_ENABLE 1
#define RTMC_MODE_DISABLE 0
#define ONLY_INERTIA 0
#define INERTIA_AND_ECC 1

#define STANDARD_IO_HIGH 1
#define STANDARD_IO_LOW 0
#define STANDARD_IO_BYTE_MAX 255
#define STANDARD_DO_BIT_RANGE_MAX 15
#define STANDARD_DO_BYTE_RANGE_MAX 1
#define STANDARD_DI_BIT_RANGE_MAX 23
#define STANDARD_DI_BYTE_RANGE_MAX 2
#define STANDARD_DI_WORD_RANGE_MAX 1


#define UNAUTHENTICAITION_PASSWORD 1
#define RC_OPTION_DISABLE 2
#define ASSIGEND_REMOTEIO 1
#define MOTOR_POWER_OFF 1
#define STOP_PROGRESS 2
#define RB_MODEL_UNMATCH 0 
#define RB_MODEL_MATCH 1 

#define MAKEWORD(l,h)                       ((unsigned int) (((unsigned int) (((unsigned int) (l)) & 0xffffffff)) | ((unsigned int) ((unsigned int) (((unsigned int) (h)) & 0xffffffff)))<<((unsigned int)16)))
#define HIWORD(hl)                           ((unsigned short) ((((unsigned int) (hl)) >> 16) & 0xffff))
#define LOWORD(hl)                           ((unsigned short) (((unsigned int) (hl)) & 0xffff))
#define MAKEUSHORT(l,h)                      ((unsigned short) (((unsigned short) (((unsigned short) (l)) & 0xffff)) | ((unsigned short) ((unsigned short) (((unsigned short) (h)) & 0xffff)))<<((unsigned short)8)))
#define HIBYTE(hl)                           ((uint8_t) ((((unsigned short) (hl)) >> 8) & 0xff))
#define LOWBYTE(hl)                           ((uint8_t) (((unsigned short) (hl)) & 0xffff))

