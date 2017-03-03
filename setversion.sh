#!/bin/sh

set -x
set -e

X_MODULES=`cat X_MODULES`
cd app/include
X_COMMIT_ID=$(git rev-parse HEAD)
X_SSL_ENABLED=true
echo "SSL enabled:" "${X_SSL_ENABLED}";

if [ "${X_SSL_ENABLED}" = "true" ]; then
  echo "Enabling SSL in user_config.h"
  sed -e 's/\/\/ *#define CLIENT_SSL_ENABLE/#define CLIENT_SSL_ENABLE/g' user_config.h > user_config.h.tmp;
else
  echo "Disabling SSL in user_config.h"
  sed -e 's/#define CLIENT_SSL_ENABLE/\/\/ #define CLIENT_SSL_ENABLE/g' user_config.h > user_config.h.tmp;
fi
mv user_config.h.tmp user_config.h;

X_BRANCH="dev"
sed 's/#define NODE_VERSION[^_].*/#define NODE_VERSION "NodeMCU custom build by apwiede\\n\\tbranch: '$X_BRANCH'\\n\\tcommit: '$X_COMMIT_ID'\\n\\tSSL: '$X_SSL_ENABLED'\\n\\tmodules: '$X_MODULES'\\n"/g' user_version.h > user_version.h.tmp && mv user_version.h.tmp user_version.h
sed 's/#define BUILD_DATE.*/#define BUILD_DATE "\\tbuilt on: '"$(date "+%Y-%m-%d %H:%M")"'\\n"/g' user_version.h > user_version.h.tmp && mv user_version.h.tmp user_version.h
cd ../..
rm X_MODULES
