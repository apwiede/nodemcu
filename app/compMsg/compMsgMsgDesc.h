/*
* Copyright (c) 2016, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
* All rights reserved.
*
* License: BSD/MIT
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/* 
 * File:   compMsgMsgDesc.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

/* composite message data descriptions handling */

/* ==========================================================================
 * Layout of message description files:
 *   #< ><text>                           comment line (possible every where)
 *   #,<number lines>[,<action callback>] number of lines in this file
 *                                        eventually a callback for an action to call
 *                                        before building this message
 *   @<special field name>,<type>,<lgth>[,<R/W>[,@<field size callback>[,<runAction>]]]
                                          special fields
 *   @#<key/value field name>,<type>,<lgth>,[<R/W>[,@<field size callback>][,@<runAction>]
 *                                        (see key/value fields description)
 *   <field name>,<type>,<lgth>[,<R/W>[,@<field size callback>[,@<runAction>]]]
                                          normal fields
 *   @$<field file name type>,<file name>[,<cmdKey>]
 *
 * Example:
 *
 * Example MsgDescA1.txt
 *   # this is an example message description file
 *   #,7,@getAPList
 *   @numKeyValues,unit8_t,1
 *   @#sequenceNum,uint32_t,4,0,@getSequenceNumSize
 *   @#ssid,uint8_t*,0,0,@getWifiAPSssidSize
 *   @#channel,uint8_t*,0,0,@getChannelSize
 *
 * Example MsgDescHeader.txt
 *   # this is an example message header file
 *   @src,uint8_t,1
 *   @grp,uint8_t,1
 *   @dst,uint8_t,1
 *   @ipAddr,uint32_t,4
 *   @totalLgth,uint16_t,2
 *   @hdrFiller,uint8_t*,7
 *
 * Example MsgDescMidPart.txt
 *   # this is an example message mid part file
 *   @cmdKey,uint16_t,2
 *   @cmdLgth,uint16_t,2
 *
 * Example MsgDescTrailer.txt
 *   # this is an example message trailer file
 *   @filler,uint8_t*,0
 *   @crc,uint16_t,2
 *   @totalCrc,uint16_t,2
 *
 * ==========================================================================
 */

/* ==========================================================================
 * Layout of message value files:
 *   #< ><text>                           comment line (possible every where)
 *   #,<number lines>[,<action callback>] number of lines in this file
 *                                        eventually a callback for an action to call
 *                                        before building this message
 *   @<special field name>,<value/callback>
 *                                        special fields
 *   @#<key/value field name>,<value/callback>
 *                                        (see key/value fields description)
 *   <field name>,<value/callback>
 *                                        normal field
 *
 * Example MsgValA1.txt
 *   # this is an example message value file
 *   #,4
 *   @numKeyValues,3
 *   @#sequenceNum,@getClientSequenceNum
 *   @#ssid,@getWifiAPSsids
 *   @#channel,@getWifiAPChannels
 *
 * Example MsgValHeaderP1.txt
 *   # this is an example message value header file
 *   #,5
 *   @src,12
 *   @grp,2
 *   @dst,15
 *   @srcId,@getSrcId
 *   @hdrReserve,@getHdrReserve
 *
 * ==========================================================================
 */

#ifndef COMP_MSG_MSG_DESC_H
#define	COMP_MSG_MSG_DESC_H

#define DISP_BUF_LGTH 1024

// handle types
// A/G/R/S/W/U/N
#define COMP_DISP_SEND_TO_APP       'A'
#define COMP_DISP_RECEIVE_FROM_APP  'G'
#define COMP_DISP_SEND_TO_UART      'R'
#define COMP_DISP_RECEIVE_FROM_UART 'S'
#define COMP_DISP_TRANSFER_TO_UART  'W'
#define COMP_DISP_TRANSFER_TO_CONN  'U'
#define COMP_DISP_NOT_RELEVANT      'N'

// encryption
enum compMsgEncyptedCode
{
  COMP_DISP_IS_NOT_ENCRYPTED  = 0,
  COMP_DISP_IS_ENCRYPTED      = 1,
  COMP_DISP_U8_ENCRYPTION     = 2,
  COMP_DISP_U8_HANDLE_TYPE    = 4,
};

#define COMP_MSG_HDR_DST              0x01
#define COMP_MSG_HDR_SRC              0x02
#define COMP_MSG_HDR_GROUP            0x04
#define COMP_MSG_HDR_TOTAL_LGTH       0x08
#define COMP_MSG_HDR_GUID             0x10
#define COMP_MSG_HDR_IP_ADDR          0x20
#define COMP_MSG_HDR_FILLER           0x40
#define COMP_MSG_PAYLOAD_CMD_KEY      0x80
#define COMP_MSG_PAYLOAD_CMD_LGTH     0x100
#define COMP_MSG_PAYLOAD_CRC          0x200
#define COMP_MSG_TOTAL_CRC            0x400

// the next value must be equal to the number of defines above!!
#define COMP_MSG_MAX_SEQUENCE         11

#define COMP_MSG_DST_U8               1
#define COMP_MSG_DST_U16              2
#define COMP_MSG_SRC_U8               3
#define COMP_MSG_SRC_U16              4
#define COMP_MSG_GROUP_U8             5
#define COMP_MSG_GROUP_U16            6
#define COMP_MSG_TOTAL_LGTH_U8        7
#define COMP_MSG_TOTAL_LGTH_U16       8
#define COMP_MSG_IP_ADDR              9
#define COMP_MSG_GUID_VECTOR          10
#define COMP_MSG_HDR_FILLER_VECTOR    12
#define COMP_MSG_CMD_KEY_U8           13
#define COMP_MSG_CMD_KEY_U16          14
#define COMP_MSG_CMD_LGTH_U0          15
#define COMP_MSG_CMD_LGTH_U8          16
#define COMP_MSG_CMD_LGTH_U16         17
#define COMP_MSG_CRC_U0               18
#define COMP_MSG_CRC_U8               19
#define COMP_MSG_CRC_U16              20
#define COMP_MSG_TOTAL_CRC_U0         21
#define COMP_MSG_TOTAL_CRC_U8         22
#define COMP_MSG_TOTAL_CRC_U16        23

#define COMP_MSG_DESC_VALUE_IS_NUMBER (1 << 0)

#define COMP_MSG_VAL_IS_JOKER          0x01

#define MSG_GUID_LGTH 16
#define MSG_MAX_HDR_FILLER_LGTH 16

#define MSG_FILES_FILE_NAME           "MsgFiles.txt"

typedef struct msgHeaderInfo {
  uint8_t numHeaderFields;
  uint16_t headerLgth;
  uint8_t *headerFieldIds;
} msgHeaderInfo_t;

typedef struct msgHeader {
  uint32_t headerKey;
  dataValue_t *dataValues;
} msgHeader_t;

typedef struct keyValueDesc {
  uint16_t keyId;
  uint8_t keyAccess;
  uint8_t keyType;
  uint16_t keyLgth;
  uint16_t keyNumValues;
} keyValueDesc_t;




typedef struct msgHeaders {
  int numMsgHeaderInfos;
  int maxMsgHeaderInfos;
  msgHeader_t *msgHeaderInfos;
} msgHeaders_t;

// infos about the fields sequence within a mesgDescription

typedef struct msgFieldSequence {
  uint16_t fieldId;
  uint16_t fieldType;
  uint8_t isJoker;
} msgFieldSequence_t;

// infos about a message with the message description infos

typedef struct msgDescription {
  uint16_t headerLgth;
  uint8_t encrypted;
  uint8_t handleType;
  msgFieldSequence_t fieldSequence[COMP_MSG_MAX_SEQUENCE];
} msgDescription_t;

// infos about all possible messages used here

typedef struct msgDescriptionInfos {
  uint8_t numMsgDescriptions;
  uint8_t maxMsgDescriptions;
  msgDescription_t *msgDescriptions;
  uint8_t currMsgDescriptionIdx;
  uint8_t currSequenceIdx;
  uint8_t sequenceIdxAfterHeader;
} msgDescriptionInfos_t;

// infos about a key/value field

typedef struct msgKeyValueDesc {
  uint16_t keyId;
  uint8_t keyAccess;
  uint8_t keyType;
  uint16_t keyLgth;
  uint16_t keyNumValues;
} msgKeyValueDesc_t;

// all infos about a message field

typedef struct msgFieldDesc {
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint16_t fieldLgth;
  uint16_t fieldFlags;
  msgKeyValueDesc_t *msgKeyValueDesc;
  fieldSizeCallback_t fieldSizeCallback;
  dataValue_t *fieldValue;
} msgFieldDesc_t;

// all infos about a message fieldGroup

typedef struct msgFieldGroupInfo {
  uint16_t fieldGroupId;
  uint16_t cmdKey;
  char *fileName;
  uint8_t maxMsgFieldDesc;
  uint8_t numMsgFieldDesc;
  msgFieldDesc_t *msgFieldDescs;
  uint8_t maxDataValues;
  uint8_t numMsgDataValues;
  dataValue_t *dataValues;
} msgFieldGroupInfo_t;







typedef struct msgInfo {
  size_t lgth;
  uint8_t realLgth;
  size_t fieldOffset;
  uint8_t buf[DISP_BUF_LGTH];
  compMsgDataView_t *compMsgDataView;
} msgInfo_t;

typedef struct fieldDesc {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldTypeStr;
  uint8_t fieldTypeId;
  uint16_t fieldLgth;
  uint16_t fieldSize;
  keyValueDesc_t *keyValueDesc;
  fieldSizeCallback_t fieldSizeCallback;
} fieldDesc_t;

#define COMP_DISP_HDR_DST              0x01
#define COMP_DISP_HDR_SRC              0x02
#define COMP_DISP_HDR_TOTAL_LGTH       0x04
#define COMP_DISP_HDR_GUID             0x08
#define COMP_DISP_HDR_SRC_ID           0x10
#define COMP_DISP_HDR_RESERVE          0x20
#define COMP_DISP_HDR_FILLER           0x40
#define COMP_DISP_PAYLOAD_CMD_KEY      0x80
#define COMP_DISP_PAYLOAD_CMD_LGTH     0x100
#define COMP_DISP_PAYLOAD_CRC          0x200
#define COMP_DISP_TOTAL_CRC            0x400

#define COMP_DISP_U16_DST              0x01
#define COMP_DISP_U16_SRC              0x02
#define COMP_DISP_U16_TOTAL_LGTH       0x04
#define COMP_DISP_U8_VECTOR_GUID       0x08
#define COMP_DISP_U16_SRC_ID           0x10
#define COMP_DISP_HDR_RESERVE          0x20
#define COMP_DISP_U8_VECTOR_HDR_FILLER 0x40
#define COMP_DISP_U16_CMD_KEY          0x80
#define COMP_DISP_U0_CMD_LGTH          0x100
#define COMP_DISP_U8_CMD_LGTH          0x200
#define COMP_DISP_U16_CMD_LGTH         0x400
#define COMP_DISP_U0_CRC               0x800
#define COMP_DISP_U8_CRC               0x1000
#define COMP_DISP_U16_CRC              0x2000
#define COMP_DISP_U0_TOTAL_CRC         0x4000
#define COMP_DISP_U8_TOTAL_CRC         0x8000
#define COMP_DISP_U16_TOTAL_CRC        0x10000

// the next value must equal the number of defines above!!
#define COMP_DISP_MAX_SEQUENCE     17

#define COMP_DISP_DESC_VALUE_IS_NUMBER (1 << 0)

#define DISP_GUID_LGTH 16
#define DISP_MAX_HDR_FILLER_LGTH 40
typedef struct headerPart {
  uint16_t hdrFromPart;
  uint16_t hdrToPart;
  uint16_t hdrTotalLgth;
  uint8_t hdrGUID[DISP_GUID_LGTH+1];
  uint16_t hdrSrcId;
  uint8_t hdrReserve[3];
  uint8_t hdrFiller[DISP_MAX_HDR_FILLER_LGTH+1];
  uint16_t hdrU16CmdKey;
  uint16_t hdrU16CmdLgth;
  uint16_t hdrU16Crc;
  uint16_t hdrU16TotalCrc;
  uint8_t hdrTargetPart;
  uint8_t hdrU8CmdKey;
  uint8_t hdrU8CmdLgth;
  uint8_t hdrU8Crc;
  uint8_t hdrU8TotalCrc;
  uint8_t hdrOffset;
  uint8_t hdrEncryption;
  uint8_t hdrHandleType;
  uint8_t hdrLgth;
  uint32_t hdrFlags;
  uint32_t fieldSequence[COMP_DISP_MAX_SEQUENCE];
} headerPart_t;

typedef struct msgHeaderInfos {
  uint32_t headerFlags;        // these are the flags for the 2nd line in the heads file!!
  uint16_t headerSequence[COMP_DISP_MAX_SEQUENCE];  // this is the sequence of the 2nd line in the heads file!!
  uint8_t headerLgth;
  size_t lgth;
  headerPart_t *headerParts;
  uint8_t numHeaderParts;
  uint8_t maxHeaderParts;
  uint8_t currPartIdx;
  uint8_t seqIdx;
  uint8_t seqIdxAfterHeader;
} msgHeaderInfos_t;

typedef struct msgParts {
  uint16_t fromPart;
  uint16_t toPart;
  uint16_t totalLgth;
  uint8_t GUID[DISP_GUID_LGTH];
  uint32_t srcId;
  uint16_t hdrReserve;
  uint8_t hdrFiller[DISP_MAX_HDR_FILLER_LGTH];
  uint16_t partsFlags;
  uint16_t u16CmdKey;
  uint8_t u8CmdLgth;
  uint16_t u16CmdLgth;
  size_t lgth;
  uint8_t encryption;
  uint8_t realLgth;
  size_t fieldOffset;
  uint8_t buf[DISP_BUF_LGTH];
  compMsgDataView_t *compMsgDataView;
} msgParts_t;


typedef struct msgDescPart {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldTypeStr;
  uint8_t fieldTypeId;
  uint8_t fieldLgth;
  uint8_t fieldType;
  uint8_t fieldRW;
  uint16_t fieldKey;
  uint16_t fieldSize;
  uint16_t fieldNumValues;
  fieldValueCallback_t fieldSizeCallback;
  uint8_t fieldValueCallbackType;
  uint8_t *fieldValueActionCb; // the name of a callback to run an action
} msgDescPart_t;

typedef struct msgValPart {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldValueStr;    // the value or the callback for getting the value
  uint8_t *fieldKeyValueStr; // the value for a string
  uint32_t fieldValue;       // the value for an integer
  uint8_t fieldFlags;
  uint8_t fieldValueCallbackType;
  fieldValueCallback_t fieldValueCallback;
} msgValPart_t;

typedef struct msgKeyValueDescPart {
  uint8_t *keyNameStr;
  uint16_t keyId;
  uint8_t keyType;
  uint8_t keyLgth;
} msgKeyValueDescPart_t;

typedef struct fieldsToSave {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldValueStr;    // the value
  uint32_t fieldValue;       // the value for an integer
} fieldsToSave_t;

typedef struct compMsgMsgDesc compMsgMsgDesc_t;
typedef struct compMsgData compMsgData_t;
typedef struct compMsgWifiData compMsgWifiData_t;

typedef uint8_t (* addHeaderInfo_t)(compMsgDispatcher_t *self, uint16_t fieldLgth, uint8_t fieldId);

typedef uint8_t (* addFieldGroup_t)(compMsgDispatcher_t *self, char *fileName, uint16_t fieldGroupId, uint16_t cmdKey);
typedef uint8_t (* handleMsgFileNameLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgHeadsLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgFieldGroupLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgCommonLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgActionsLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgValuesLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgFieldsToSaveLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgLine_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleMsgFile_t)(compMsgDispatcher_t *self, uint8_t *fileName, handleMsgLine_t handleMsgLine);

typedef uint8_t (* compMsgMsgDescInit_t)(compMsgDispatcher_t *self);
typedef void (* freeCompMsgMsgDesc_t)(compMsgMsgDesc_t *compMsgMsgDesc);

typedef struct compMsgMsgDesc {
  int expectedLines;
  msgDescriptionInfos_t msgDescriptionInfos;
  int numMsgFieldGroupInfo;
  int maxMsgFieldGroupInfo;
  int currMsgFieldGroupInfo;
  msgFieldGroupInfo_t *msgFieldGroupInfos;
  uint8_t *msgFieldGroupFileName;

  msgHeaderInfo_t msgHeaderInfo;
  addHeaderInfo_t addHeaderInfo;
  addFieldGroup_t addFieldGroup;

  handleMsgFile_t handleMsgFile;
  handleMsgFieldGroupLine_t handleMsgFieldGroupLine;
  handleMsgCommonLine_t handleMsgCommonLine;
  handleMsgFieldsToSaveLine_t handleMsgFieldsToSaveLine;
  handleMsgActionsLine_t handleMsgActionsLine;
  handleMsgValuesLine_t handleMsgValuesLine;
  handleMsgHeadsLine_t handleMsgHeadsLine;
  handleMsgFileNameLine_t handleMsgFileNameLine;

  freeCompMsgMsgDesc_t freeCompMsgMsgDesc;
  compMsgMsgDescInit_t compMsgMsgDescInit;

} compMsgMsgDesc_t;

compMsgMsgDesc_t *newCompMsgMsgDesc();

#endif	/* COMP_MSG_MSG_DESC_H */
