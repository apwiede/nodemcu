// ================================= getPingState ====================================

static uint8_t getPingState(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.PingState;
  *numericValue = compMsgModuleData.PingState;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getDeviceSecurity ====================================

static uint8_t getDeviceSecurity(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.DeviceSecurity;
  *numericValue = compMsgModuleData.DeviceSecurity;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProgRunningMode ====================================

static uint8_t getProgRunningMode(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ProgRunningMode;
  *numericValue = compMsgModuleData.ProgRunningMode;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= CurrentProgRunningMode ====================================

static uint8_t getCurrentRunningMode(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.CurrentRunningMode;
  *numericValue = compMsgModuleData.CurrentRunningMode;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getRegion ====================================

static uint8_t getRegion(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.Region;
  *numericValue = compMsgModuleData.Region;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getSerieNumber ====================================

static uint8_t getSerieNumber(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.SerieNumber;
  *numericValue = 0;
  *stringValue = compMsgModuleData.SerieNumber;
  return COMP_MSG_ERR_OK;
}

// ================================= getHdrReserve ====================================

static uint8_t getHdrReserve(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
//  if (self->compMsgData->msgValPart->fieldValueStr != NULL) {
//    os_free(self->compMsgData->msgValPart->fieldValueStr);
//  }
//  self->compMsgData->msgValPart->fieldValueStr = compMsgModuleData.hdrReserve;
  *numericValue = 0;
  *stringValue = compMsgModuleData.hdrReserve;
  return COMP_MSG_ERR_OK;
}

// ================================= getPasswdC ====================================

static uint8_t getPasswdC(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.passwdC;
  *numericValue = 0;
  *stringValue = compMsgModuleData.passwdC;
  return COMP_MSG_ERR_OK;
}

// ================================= setOperatingMode ====================================

static uint8_t setOperatingMode(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  int result;
  fieldValue_t fieldValue;

  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = self->dispatcherCommon->operatingMode;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_operatingMode;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->setFieldValueInfo(self, &fieldValue);
  return COMP_MSG_ERR_OK;
}

// ================================= getMyU16Src ====================================

static uint8_t getMyU16Src(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.myU16Src;
  *numericValue = compMsgModuleData.myU16Src;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getMyU8Src ====================================

static uint8_t getMyU8Src(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.myU8Src;
  *numericValue = compMsgModuleData.myU8Src;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getMyU16SaveUserDataCmdKey ====================================

static uint8_t getMyU16SaveUserDataCmdKey(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.myU16SaveUserDataCmdKey;
  *numericValue = compMsgModuleData.myU16SaveUserDataCmdKey;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getMyU8SaveUserDataCmdKey ====================================

static uint8_t getMyU8SaveUserDataCmdKey(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.myU8SaveUserDataCmdKey;
  *numericValue = compMsgModuleData.myU8SaveUserDataCmdKey;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getSrcId ====================================

static uint8_t getSrcId(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.srcId;
  *numericValue = compMsgModuleData.srcId;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getGUID ====================================

static uint8_t getGUID(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.GUID;
  *numericValue = 0;
  *stringValue = compMsgModuleData.GUID;
  return COMP_MSG_ERR_OK;
}


// ================================= customInit  ====================================

static uint8_t customInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgUtil_t *compMsgUtil;

  compMsgUtil = self->compMsgUtil;
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_PingState, &getPingState);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_DeviceSecurity, &getDeviceSecurity);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_ProgRunningMode, &getProgRunningMode);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_CurrentRunningMode, &getCurrentRunningMode);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_Region, &getRegion);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_SerieNumber, &getSerieNumber);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_HdrReserve, &getHdrReserve);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_PasswdC, &getPasswdC);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_OperatingMode, &setOperatingMode);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_MyU16Src, &getMyU16Src);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_MyU8Src, &getMyU8Src);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_MyU16SaveUserDataCmdKey, &getMyU16SaveUserDataCmdKey);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_MyU8SaveUserDataCmdKey, &getMyU8SaveUserDataCmdKey);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_SrcId, &getSrcId);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DATA_GUID, &getGUID);
  checkErrOK(result);
  return result;
}
