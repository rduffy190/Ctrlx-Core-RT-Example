#!/bin/bash
# shellcheck disable=SC1090,SC1091,SC2044

# Exit immediately if a command exits with a non-zero status.
set -e

ARCH=arm64

ADDR=192.168.1.1
SSL_PORT=443
SSH_PORT=22

ASK=n

CONNECT_TIMEOUT=5
COUNTER_MAX=60

SECONDS_TO_WAIT_AFTER_SHOW_ARGUMENTS=5
SECONDS_TO_WAIT_AFTER_UPLOAD=30
SECONDS_TO_WAIT_AFTER_UNINSTALL=30
SECONDS_TO_WAIT_PROGRESS=2

BUILD=y
REMOTE_DEBUG=y
UPLOAD=y
UNINSTALL=n

SSL_USR=boschrexroth
SSL_PWD=boschrexroth

SERVICE=y
OPERATING=y
AUTOMATIC=y

SSH_USR=rexroot
SSH_PWD=rexroot

GDB_PORT=12345

LOGS=y

# Function(s) ------------------------------------------------------------------------------
function WaitForTaskFinished() {
	# wait before first try
	# sleep 10

	TASK_ID=$1

	echo -----------------------------------------------------------------
	echo -n "Wait for task $TASK_ID is finished "

	# uncomment for debugging
	# echo "----- DEBUG -----"
	# echo "$TASK_ID"
	# echo "$HOST"
	# echo "----- DEBUG -----"

	COUNTER=0
	PROGRESS_TOTAL=0
	while true; do

		RESPONSE=$(curl -X GET https://"${ADDR}":"${SSL_PORT}"/package-manager/api/v1/tasks/"$TASK_ID" \
			-H "accept: application/json" \
			-H "Authorization: Bearer ${TOKEN}" \
			--insecure --silent --connect-timeout ${CONNECT_TIMEOUT})

		snap=$(echo "$RESPONSE" | jq -r .parameters.id)
		action=$(echo "$RESPONSE" | jq -r .action)
		state=$(echo "$RESPONSE" | jq -r .state)
		result_detail=$(echo "$RESPONSE" | jq -r .result.detail)

		# uncomment for debugging
		# echo "----- DEBUG -----"
		# echo "$RESPONSE"
		# echo "----- DEBUG -----"

		if [[ "$state" == "failed" ]]; then
			echo "ERROR action '$action' of '$snap' is failed: $result_detail"
			return
		fi

		if [[ "$state" == "pending" ]]; then
			echo -n "."
		fi

		if [[ "$state" == "running" ]]; then
			PROGRESS=$(echo "$RESPONSE" | jq .progress)
			if [[ "$PROGRESS" != null ]] && test "$PROGRESS" != "$PROGRESS_TOTAL"; then
				echo -n ". $PROGRESS% ."
				PROGRESS_TOTAL=$PROGRESS
			else
				echo -n "."
			fi
		fi

		if [[ "$state" == "done" ]]; then
			echo " done"
			return
		fi

		COUNTER=$((COUNTER + 1))
		if [[ $COUNTER == "$COUNTER_MAX" ]]; then
			echo FAILED
			echo "ERROR action '$action' of '$snap' is timed out!"
			return
		fi

		# wait for next try
		sleep $SECONDS_TO_WAIT_PROGRESS
	done

}

function FindSnap() {
	SNAP_PATH=$(find . -name "*_${ARCH}.snap" -exec ls -t1 {} + | head -1)
	if [ -z "${SNAP_PATH}" ]; then
		echo "ERROR No snap file found"
		exit 1
	fi

	SNAP_FILE=$(basename "${SNAP_PATH}")
	SNAP_NAME=$(echo "${SNAP_FILE}" | cut -d'_' -f 1)

	echo "snap file: ${SNAP_FILE}"
	echo "snap name: ${SNAP_NAME}"
}

function ActivateSshAccess() {

	echo ""
	echo -----------------------------------------------------------------
	echo Activate SSH access
	echo ""

	curl -X 'PUT' \
		https://"${ADDR}":"${SSL_PORT}"/ssh/api/v1/status \
		-H 'accept: application/json' \
		-H "Authorization: Bearer ${TOKEN}" \
		-H 'Content-Type: application/json' \
		-d '{"state": "activated"}' \
		--insecure --no-progress-meter --silent --show-error

	echo ""

	ssh-keygen -f "/home/boschrexroth/.ssh/known_hosts" -R "[${ADDR}]:${SSH_PORT}" >/dev/null 2>&1
}

function SwitchScheduler() {
	
	local path=$1
	local key=$2 
	local value=$3

	echo ""
	echo -----------------------------------------------------------------
	echo -n "Switching Scheduler $key=$value "

	local url=https://"${ADDR}":"${SSL_PORT}"/automation/api/v1/scheduler/admin/$path
	local data={\"$key\":\"$value\"}

	for i in $(seq 1 10); do

		echo -n "."

		RESPONSE=$(curl -X 'PUT' \
			$url?format=json \
			-H 'accept: application/json' \
			-H "Authorization: Bearer ${TOKEN}" \
			-H 'Content-Type: application/json' \
			-d $data \
			--insecure --no-progress-meter --silent --show-error)

		if grep -q "$value" <<<"${RESPONSE}"; then
			# OK
			break
		fi

		sleep 5

	done

	echo ""

	if grep -q "$value" <<<"${RESPONSE}"; then
		echo ${RESPONSE}
	else
		echo "ERROR Switching Scheduler $path $key=$value failed."
		read -r -t ${SECONDS_TO_WAIT_AFTER_UPLOAD} -p "Wait or press ENTER to continue"
	fi
}

function SwitchSchedulerToStateService() {
	SwitchScheduler state state SERVICE
}


function SwitchSchedulerToStateOperating() {
	SwitchScheduler state state OPERATING
}

function SwitchSchedulerToModeAutomatic() {
	SwitchScheduler cfg/program-configuration mode AUTOMATIC
}

# Scanning parameter list

i=0
for arg in "$@"; do
	i=$((i + 1))
	argNext=$((i + 1))

	if grep -q "ask" <<<"${arg}"; then
		ASK=y
	fi

	if grep -q "arch" <<<"${arg}"; then
		ARCH=${!argNext}
	fi

	if grep -q "NA" <<<"${arg}"; then
		ARCH=amd64
	fi

	if grep -q "PF" <<<"${arg}"; then
		ADDR=10.0.2.2
		SSL_PORT=8443
		ARCH=amd64
		SSH_PORT=8022
	fi

	if grep -q "build" <<<"${arg}"; then
		BUILD=${!argNext}
	fi

	if grep -q "remote-debug" <<<"${arg}"; then
		REMOTE_DEBUG=${!argNext}
	fi

	if grep -q "upload" <<<"${arg}"; then
		UPLOAD=${!argNext}
	fi

	if grep -q "uninst" <<<"${arg}"; then
		UNINSTALL=y
	fi

	if grep -q "addr" <<<"${arg}"; then
		ADDR=${!argNext}
	fi

	if grep -q "ssl-port" <<<"${arg}"; then
		SSL_PORT=${!argNext}
	fi

	if grep -q "ssl-usr" <<<"${arg}"; then
		SSL_USR=${!argNext}
	fi

	if grep -q "ssl-pwd" <<<"${arg}"; then
		SSL_PWD=${!argNext}
	fi

	if grep -q "gdb-port" <<<"${arg}"; then
		GDB_PORT=${!argNext}
	fi

	if grep -q "serv" <<<"${arg}"; then
		SERVICE=${!argNext}
	fi

	if grep -q "oper" <<<"${arg}"; then
		OPERATING=${!argNext}
	fi

	if grep -q "auto" <<<"${arg}"; then
		AUTOMATIC=${!argNext}
	fi

	if grep -q "arch" <<<"${arg}"; then
		ARCH=${!argNext}
	fi

	if grep -q "log" <<<"${arg}"; then
		LOGS=${!argNext}
	fi

	if grep -q "ssh-port" <<<"${arg}"; then
		SSH_PORT=${!argNext}
	fi

	if grep -q "ssh-usr" <<<"${arg}"; then
		SSH_USR=${!argNext}
	fi

	if grep -q "ssh-pwd" <<<"${arg}"; then
		SSH_PWD=${!argNext}
	fi

	if grep -q "help" <<<"${arg}"; then
		echo ""
		echo "-----------------------------------------------------"
		echo "Build, upload and show snap logs"
		echo ""
		echo "Usage:"
		echo "build-upload-log-snap.sh  - Use default values for ctrlX CORE M3/4"
		echo ""
		echo "All command line parameters with their default value:"
		echo "-----------------------------------------------------"
		echo "-build          Build snap"
		echo "  y             Default: Include snap build"
		echo "  n             Skip snap build"
		echo ""
		echo "-remote-debug   Build snap in debug mode, start remote debug server"
		echo ""
		echo "-upload         Upload snap"
		echo "  y             Default: Upload"
		echo "  n             Skip upload"
		echo ""
		echo "-uninstall      Uninstall snap"
		echo ""
		echo "-addr           IP address or name of the destination ctrlX CORE"
		echo "  192.168.1.1   Default: ctrlX CORE"
		echo "  10.0.2.2      ctrlX COREvirtual with port forwarding"
		echo ""
		echo "-ssl-port       Port number for HTTPS (SSL) connection"
		echo "  443           Default: ctrlX CORE"
		echo "  8443          ctrlX COREvirtual with port forwarding"
		echo ""
		echo "-ssl-usr        User name for HTTPS (SSL) connection"
		echo "  boschrexroth  default value"
		echo ""
		echo "-ssl-pwd        Password for HTTPS (SSL) connection"
		echo "  boschrexroth  Default value"
		echo ""
		echo "-service        Switch scheduler to state Service"
		echo "  n             Default: Do not switch"
		echo "  y             Switch to SERVICE"
		echo ""
		echo "-operating      Switch scheduler to state Operating"
		echo "  n             Default: Do not switch"
		echo "  y             Switch to OPERATING"
		echo ""
		echo "-automatic      Switch scheduler to mode Automatic"
		echo "  y             Default: Switch to Automatic mode"
		echo "  n             Do not switch"
		echo ""
		echo "-arch           CPU architecture"
		echo "  arm64         Default: CORE X3"
		echo "  amd64         ctrlX COREvirtual, OS"
		echo ""
		echo "-ssh-port       Port number for HTTPS (SSL) connection"
		echo "  22            Default: ctrlX CORE"
		echo "  8022          ctrlX COREvirtual with port forwarding"
		echo ""
		echo "-ssh-usr        User name for SSH connection"
		echo "  rexroot       Default value"
		echo ""
		echo "-gdb-port       Port number of gdb server"
		echo "  12345         Default value"
		echo ""
		echo ""
		echo "-PF             Use settings for ctrlX COREvirtual with Port Forwarding"
		echo "                10.0.2.2, 8443, amd64, 8022"
		echo ""
		echo "-NA             Use settings for ctrlX COREvirtual with Network Adapter"
		echo "                192.168.1.1, 443, amd64, 22"
		echo ""

		exit 1
	fi
done

# Derive bundle name
BUNDLE=$(ls generated/bundle | sed -r 's/\./_/g' )
BUNDLE=$(echo $BUNDLE | sed -r 's/_zip//g' )

# Dependend settings
if grep -q "y" <<<"${REMOTE_DEBUG}"; then
	OPERATING=y
	LOGS=n
fi

if grep -q "y" <<<"${AUTOMATIC}"; then
	# Automatic mode requires state SERVICE
	SERVICE=y
fi

echo ""
echo Bundle $BUNDLE
echo -arch "${ARCH}"
echo -addr "${ADDR}"
echo -ssl-port "${SSL_PORT}"
echo -ssl-usr "${SSL_USR}"
echo -ssl-pwd "${SSL_PWD}"
echo -ssh-port "${SSH_PORT}"
echo ""
echo -build "${BUILD}"
echo -remote-debug "${REMOTE_DEBUG}"
echo ""
echo -upload "${UPLOAD}"
echo -uninstall "${UNINSTALL}"
echo ""
echo -service "${SERVICE}"
echo -operating "${OPERATING}"
echo -automatic "${AUTOMATIC}"
echo ""
echo -logs "${LOGS}"
echo ""
echo ""

if grep -q "y" <<<"${ASK}"; then
	echo "Settings OK? Waiting" ${SECONDS_TO_WAIT_AFTER_SHOW_ARGUMENTS} "s ..."
	echo "Press Ctrl-C to abort"
	sleep ${SECONDS_TO_WAIT_AFTER_SHOW_ARGUMENTS}
fi

if grep -q "y" <<<"${BUILD}"; then
	echo ""
	echo -----------------------------------------------------------------
	echo Building snap
	echo ""
	
	if grep -q "y" <<<"${REMOTE_DEBUG}"; then
		source build-debug-snap-"${ARCH}".sh
	else
		source build-snap-"${ARCH}".sh
	fi
fi

echo ""
echo -----------------------------------------------------------------
echo Requesting new Bearer Token
echo ""

RESPONSE=$(curl --insecure --no-progress-meter --request POST \
	https://"${ADDR}":"${SSL_PORT}"/identity-manager/api/v2/auth/token?force-local-auth=false \
	--header 'Content-Type: application/json' \
	--data-raw '{"name":"'"${SSL_USR}"'","password":"'"${SSL_PWD}"'"}')
	
# https://stackoverflow.com/questions/9733338/shell-script-remove-first-and-last-quote-from-a-variable
# Remove first and last quote (") from a variable
TOKEN=$(echo "${RESPONSE}" | jq .access_token | xargs)

echo Bearer "${TOKEN}"
echo ""

if grep -q "y" <<<"${SERVICE}"; then
	SwitchSchedulerToStateService
fi

if grep -q "y" <<<"${AUTOMATIC}"; then
	SwitchSchedulerToModeAutomatic
fi

if grep -q "y" <<<"${UNINSTALL}"; then

	echo ""
	echo -----------------------------------------------------------------
	echo Uninstalling "${ARCH}" snaps

	# Read list of installed snaps
	ALL_INSTALLED_SNAPS=$(curl -X 'GET' \
		https://"${ADDR}":"${SSL_PORT}"/package-manager/api/v1/packages \
		-H 'accept: application/json' \
		-H "Authorization: Bearer ${TOKEN}" \
		-H 'Content-Type: application/json' \
		--insecure --no-progress-meter)

	FindSnap

	if grep -q "$SNAP_NAME" <<<"${ALL_INSTALLED_SNAPS}"; then

		echo ""
		echo Uninstalling "$SNAP_NAME" --------------------------
		echo ""

		curl -X POST \
			https://"${ADDR}":"${SSL_PORT}"/package-manager/api/v1/tasks \
			-H 'accept: */*' \
			-H "Authorization: Bearer ${TOKEN}" \
			-H 'Content-Type: application/json' \
			-d '{ "action": "uninstall", "parameters": { "id": "'"${SNAP_NAME}"'" } }' \
			--insecure --no-progress-meter --silent --show-error

		echo "Waiting ${SECONDS_TO_WAIT_AFTER_UNINSTALL}s after uninstallation ..."
		read -r -t ${SECONDS_TO_WAIT_AFTER_UNINSTALL} -p "Wait or press ENTER to continue"
		echo ""

	else
		echo NOT installed: "$SNAP_NAME"
	fi

fi

if grep -q "y" <<<"${UPLOAD}"; then

	echo ""
	echo -----------------------------------------------------------------
	echo Allow installation from unknown source
	echo ""

	curl -X PUT \
		https://"${ADDR}":"${SSL_PORT}"/package-manager/api/v1/settings \
		-H 'accept: */*' \
		-H "Authorization: Bearer ${TOKEN}" \
		-H 'Content-Type: application/json' \
		-d '{ "allowUnknownApps": true }' \
		--insecure --no-progress-meter --silent --show-error

	echo ""
	echo -----------------------------------------------------------------
	echo Uploading and installing "${ARCH} snap(s)"

	FindSnap
	echo ""

	# Sys
	LOCATION=$(curl -X POST https://"${ADDR}":"${SSL_PORT}"/package-manager/api/v1/packages \
		-H "accept: */*" \
		-H "Authorization: Bearer ${TOKEN}" \
		-H "Content-Type: multipart/form-data" \
		-F "file=@${SNAP_FILE}" \
		-F update=true \
		--insecure --silent --include | tr -d '\r' | sed -En 's/^location: (.*)/tem \1/p')

	TASK_ID=$(echo "$LOCATION" | cut -d'/' -f 3)

	# uncomment for debugging
	# echo "----- DEBUG -----"
	# echo LOCATION: "$LOCATION"
	# echo TASK_ID: "$TASK_ID"
	# echo "----- DEBUG -----"

	WaitForTaskFinished "$TASK_ID"

	echo ""
	echo -----------------------------------------------------------------
	echo Checking if ${SNAP_NAME} is installed

	# Read list of installed snaps
	ALL_INSTALLED_SNAPS=$(curl -X 'GET' \
		https://"${ADDR}":"${SSL_PORT}"/package-manager/api/v1/packages \
		-H 'accept: application/json' \
		-H "Authorization: Bearer ${TOKEN}" \
		-H 'Content-Type: application/json' \
		--insecure --no-progress-meter)

	FindSnap

	if grep -q "${SNAP_NAME}" <<<"${ALL_INSTALLED_SNAPS}"; then
		echo INFO Is installed "${SNAP_NAME}"
	else
		echo ERROR Is NOT installed "${SNAP_NAME}"
    	read -r -t ${SECONDS_TO_WAIT_AFTER_UNINSTALL} -p "Wait or press ENTER to continue"
	fi

fi

if grep -q "y" <<<"${OPERATING}"; then
	SwitchSchedulerToStateOperating
fi

echo ""
echo -----------------------------------------------------------------
echo Checking bundle "$BUNDLE"
echo ""

BUNDLES=$(curl -X 'GET' \
	https://"${ADDR}":"${SSL_PORT}"/automation/api/v2/nodes/framework/bundles?type=browse \
	-H 'accept: application/json' \
	-H "Authorization: Bearer ${TOKEN}" \
	--insecure --no-progress-meter)

if grep -q "$BUNDLE" <<<"$BUNDLES"; then

	STATE=$(curl -X 'GET' \
		https://"${ADDR}":"${SSL_PORT}"/automation/api/v2/nodes/framework/bundles/"$BUNDLE" \
		-H 'accept: application/json' \
		-H "Authorization: Bearer ${TOKEN}" \
		--insecure --no-progress-meter)

	echo "name=$(echo $STATE | jq .value.name)"
	echo "version=$(echo $STATE | jq .value.version)"
	echo "state=$(echo $STATE | jq .value.state)"
	echo "active=$(echo $STATE | jq .value.active)"
	echo "installed=$(echo $STATE | jq .value.installed)"
	echo "components="
	echo "$(echo $STATE | jq .value.components)"

else
	echo "ERROR Is NOT registered: $BUNDLE"
	read -r -t ${SECONDS_TO_WAIT_AFTER_UPLOAD} -p "Wait or press ENTER to continue"
fi

if grep -q "y" <<<"${LOGS}"; then

	ActivateSshAccess

	echo ""
	echo -----------------------------------------------------------------
	echo Viewing logs
	echo ""

#sudo snap logs -f rexroth-automationcore
	sshpass -p "${SSH_PWD}" ssh \
		-o StrictHostKeyChecking=no \
		-p "${SSH_PORT}" "${SSH_USR}@${ADDR}" \
		sudo journalctl -f
fi

if grep -q "y" <<<"${REMOTE_DEBUG}"; then

	ActivateSshAccess

	# Mandatory: Scheduler state Operating, mode Automatic
	# Start gdb-server, rexroth-automationcore is automatically stopped
	source remote-debug-run-gdb-server.sh -addr $ADDR -ssh-port $SSH_PORT -ssh-usr $SSH_USR -ssh-pwd $SSH_PWD -gdb-port $GDB_PORT
	
	# Stop gdb server, restart rexroth-automationcore
	source remote-debug-stop-gb-server.sh -addr $ADDR -ssh-port $SSH_PORT -ssh-usr $SSH_USR -ssh-pwd $SSH_PWD -gdb-port $GDB_PORT
fi

echo ""
echo -----------------------------------------------------------------
echo Deleting Bearer Token
echo ""
curl \
	--insecure --no-progress-meter \
	--request DELETE https://"${ADDR}":"${SSL_PORT}"/identity-manager/api/v1/auth/token \
	-H 'accept: */*' -H "Authorization: Bearer ${TOKEN}"