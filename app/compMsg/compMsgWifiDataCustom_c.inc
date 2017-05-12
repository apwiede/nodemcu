// ================================= getCloudPort ====================================

static uint8_t getCloudPort(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.cloudPort;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudHost1 ====================================

static uint8_t getCloudHost1(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudHost1;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudHost2 ====================================

static uint8_t getCloudHost2(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudHost2;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudUrl1Part1 ====================================

static uint8_t getCloudUrl1Part1(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudUrl1Part1;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudUrl1Part2 ====================================

static uint8_t getCloudUrl1Part2(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudUrl1Part2;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudUrlTenantId1 ====================================

static uint8_t getCloudUrlTenantId1(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudUrlTenantId1;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudUrl2Part1 ====================================

static uint8_t getCloudUrl2Part1(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudUrl2Part1;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudUrl2Part2 ====================================

static uint8_t getCloudUrl2Part2(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudUrl2Part2;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudUrlTenantId2 ====================================

static uint8_t getCloudUrlTenantId2(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudUrlTenantId2;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudNodeToken1 ====================================

static uint8_t getCloudNodeToken1(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudNodeToken1;
  return COMP_MSG_ERR_OK;
}

// ================================= getCloudNodeToken2 ====================================

static uint8_t getCloudNodeToken2(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.cloudNodeToken2;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestSsid ====================================

static uint8_t getProdTestSsid(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.prodTestSsid;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestPasswd ====================================

static uint8_t getProdTestPasswd(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.prodTestPasswd;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestSecurityType ====================================

static uint8_t getProdTestSecurityType(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.prodTestSecurityType;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestTargetProtocol ====================================

static uint8_t getProdTestTargetProtocol(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.prodTestTargetProtocol;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestIpAddress ====================================

static uint8_t getProdTestIpAddress(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.prodTestIpAddr;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestSubnet ====================================

static uint8_t getProdTestSubnet(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.prodTestSubnet;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestGateway ====================================

static uint8_t getProdTestGateway(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.prodTestGateway;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestDns ====================================

static uint8_t getProdTestDns(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.prodTestDns;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestPingAddress ====================================

static uint8_t getProdTestPingAddress(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.prodTestPingAddress;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProdTestStatus ====================================

static uint8_t getProdTestStatus(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.prodTestStatus;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= customInit  ====================================

static uint8_t customInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgUtil_t *compMsgUtil;

  compMsgUtil = self->compMsgUtil;
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_CloudPort, &getCloudPort);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_CloudHost1, &getCloudHost1);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_CloudHost2, &getCloudHost2);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_CloudUrl1Part1, &getCloudUrl1Part1);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_CloudUrl1Part2, &getCloudUrl1Part2);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_CloudUrlTenantId1, &getCloudUrlTenantId1);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_Cloudl2Part1, &getCloudUrl2Part1);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_Cloudl2Part2, &getCloudUrl2Part2);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_CloudlTenantId2, &getCloudUrlTenantId2);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_CloudNodeToken1, &getCloudNodeToken1);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_CloudNodeToken2, &getCloudNodeToken2);
  checkErrOK(result);

  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestSsid, &getProdTestSsid);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestPasswd, &getProdTestPasswd);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestSecurityType, &getProdTestSecurityType);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestTargetProtocol, &getProdTestTargetProtocol);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestIpAddress, &getProdTestIpAddress);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestSubnet, &getProdTestSubnet);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestGateway, &getProdTestGateway);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestDns, &getProdTestDns);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestPingAddress, &getProdTestPingAddress);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProdTestStatus, &getProdTestStatus);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}
