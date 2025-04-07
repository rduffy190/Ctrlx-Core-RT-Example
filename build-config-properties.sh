#!/usr/bin/env bash
# shellcheck disable=SC1090,SC1091,SC2129 

# Exit immediately if a command exits with a non-zero status.
set -e

#
# Builds the config.properties file needed to define bundles and parameters to run Celix started by rexroth-automation-framework.
#
#
# The list of bundles can be defined in the cosgi.auto.start.1=... values.
#
# IMPORTANT: The bundles are listed within one line, separated by a space (blank) character.
#
# cosgi.auto.start.1: Here found project specific bundles under generated/bundle/ are listet
# cosgi.auto.start.2: Here system bundles are listed - celix bundles first to provide a celix shell already during the startup phase .
#
# cosgi.auto.start.3:
# Here project specific required bundles can be listed. Therefor a script cosgi.auto.start.3.sh is searched in the project folder.
# For example:
#
# ADDITIONAL_BUNDLES="/home/riley/com.boschrexroth.sdk.control.rt-2.6.1/public/bundle/motion.core/${LINUX_FLAVOUR}-gcc-x64/com.boschrexroth.motion.core.zip "
# ADDITIONAL_BUNDLES="${ADDITIONAL_BUNDLES}/home/riley/com.boschrexroth.sdk.control.rt-2.6.1/public/bundle/comm.axisprofile/${LINUX_FLAVOUR}-gcc-x64/com.boschrexroth.comm.axisprofile.zip "
# echo "cosgi.auto.start.3=${ADDITIONAL_BUNDLES}" >>$CONFIG_PROPERTIES_FILE
#

# Supported: amd64, arm64
arch=$(dpkg --print-architecture)
if [[ ${arch} == "amd64" ]];then
	PLATFORM=x64
else
	PLATFORM=aarch64
fi

CONFIG_PROPERTIES_FILE=generated/config.properties
echo "Creating ${CONFIG_PROPERTIES_FILE} ===================================================="

# Start group 1: Project bundles -------------------------------------------------------------
echo "Project bundle(s) -----------------------------------------"
PROJECT_BUNDLES=""
for file in generated/bundle/*.zip; do
	PROJECT_BUNDLES="${PROJECT_BUNDLES}$file "
	echo "${file}"
done

echo "# Bundle(s) to be tested first --------------------------------------" >$CONFIG_PROPERTIES_FILE
echo "cosgi.auto.start.1=${PROJECT_BUNDLES}" >>$CONFIG_PROPERTIES_FILE


echo "System bundles --------------------------------"
UBUNTU_PLATFORM=ubuntu22-gcc-${PLATFORM}
SYSTEM_BUNDLES=""
# Add celix shell bundles first so that textual user interface (tui) is available
# celix_ShellCxx.zip is obsolete and will be removed
for file in /home/riley/com.boschrexroth.sdk.control.rt-2.6.1/public/bundle/app.automationcore/"${UBUNTU_PLATFORM}"/celix_shell*.zip; do
	echo "${file}"
	SYSTEM_BUNDLES="${SYSTEM_BUNDLES}$file "
done

# Then add system bundles
for file in /home/riley/com.boschrexroth.sdk.control.rt-2.6.1/public/bundle/app.automationcore/"${UBUNTU_PLATFORM}"/com.boschrexroth*.zip; do
	echo "${file}"
	SYSTEM_BUNDLES="${SYSTEM_BUNDLES}$file "
done

echo "# Bundles from app.automationcore --------------------------------" >>$CONFIG_PROPERTIES_FILE
echo "cosgi.auto.start.2=${SYSTEM_BUNDLES}" >>$CONFIG_PROPERTIES_FILE

# Start group 3: ADDITIONAL bundles -----------------------------------------------------------
START3_SH=cosgi.auto.start.3.sh

if [ -f "${START3_SH}" ]; then
	ADDITIONAL_BUNDLES=""
	source ${START3_SH} $UBUNTU_PLATFORM
	if [ -n "$ADDITIONAL_BUNDLES" ]; then
		echo "# Additional bundles --------------------------------" >>$CONFIG_PROPERTIES_FILE
		echo "cosgi.auto.start.3=${ADDITIONAL_BUNDLES}" >>$CONFIG_PROPERTIES_FILE
		echo "Additional bundles --------------------------------"
		echo "${ADDITIONAL_BUNDLES}" | tr ' ' '\n' 
	fi
fi

# Parameters/Options --------------------------------------------------------------------------
echo "# System options ------------------------------------------------" >>$CONFIG_PROPERTIES_FILE
# echo "oblivion.datalayerpath=framework" >>$CONFIG_PROPERTIES_FILE
# echo "bundle.observe.path=${BUNDLE_OBSERVE_PATH}" >>$CONFIG_PROPERTIES_FILE
echo "# Enable REST API" >>$CONFIG_PROPERTIES_FILE
echo "datalayer.httpport=8069" >>$CONFIG_PROPERTIES_FILE
echo "" >>$CONFIG_PROPERTIES_FILE
echo "#Disables the license check" >>$CONFIG_PROPERTIES_FILE
echo "license.proxy.debug=true" >>$CONFIG_PROPERTIES_FILE
echo "" >>$CONFIG_PROPERTIES_FILE
echo "# Disables token check of datalayer TCP connections" >>$CONFIG_PROPERTIES_FILE
echo "tokencheck.enable=false" >>$CONFIG_PROPERTIES_FILE
echo "" >>$CONFIG_PROPERTIES_FILE
echo "app.load.device.settings.enable=false" >>$CONFIG_PROPERTIES_FILE
echo "app.wait.for.device.settings.enable=false" >>$CONFIG_PROPERTIES_FILE
echo "app.restart.loop.detection.enable=false" >>$CONFIG_PROPERTIES_FILE
echo "safe.machine.state.handling.enable=false" >>$CONFIG_PROPERTIES_FILE

OPTIONS_SH=cosgi.options.sh
if [ -f "${OPTIONS_SH}" ]; then
	source ${OPTIONS_SH} $UBUNTU_PLATFORM
fi
