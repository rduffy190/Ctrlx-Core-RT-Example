#!/bin/bash
# shellcheck disable=SC1090,SC1091,SC2044

# Exit immediately if a command exits with a non-zero status.
set -e

ADDR=192.168.1.1
SSH_PORT=22

SSH_USR=rexroot
SSH_PWD=rexroot

GDB_PORT=12345


# Scanning parameter list

i=0
for arg in "$@"; do
	i=$((i + 1))
	argNext=$((i + 1))

	if grep -q "PF" <<<"${arg}"; then
		ADDR=10.0.2.2
		SSH_PORT=8022
	fi

	if grep -q "addr" <<<"${arg}"; then
		ADDR=${!argNext}
	fi

	if grep -q "gdb-port" <<<"${arg}"; then
		GDB_PORT=${!argNext}
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
		echo "Run gdb server on remote ctrlX"
		echo ""
		echo "Command line parameters with their default value:"
		echo "-----------------------------------------------------"
		echo "-addr           IP address or name of the destination ctrlX CORE"
		echo "  192.168.1.1   Default: ctrlX CORE"
		echo "  10.0.2.2      ctrlX COREvirtual with port forwarding"
		echo "-ssh-port       Port number for HTTPS (SSL) connection"
		echo "  22            Default: ctrlX CORE"
		echo "  8022          ctrlX COREvirtual with port forwarding"
		echo ""
		echo "-ssh-usr        User name for SSH connection"
		echo "  rexroot       Default value"
		echo "-ssh-pwd        Password for SSH connection"
		echo "  rexroot       Default value"
		echo ""
		echo ""
		echo "-gdb-port       Port number of gdb server"
		echo "  12345         Default value"
		echo ""
		echo "-PF             Use settings for ctrlX COREvirtual with Port Forwarding"
		echo "                10.0.2.2, 8022"
		echo ""

		exit 1
	fi
done

# Stop rexroth-automationcore.control, start rexroth-automationcore.control with gdb server
sshpass -p ${SSH_PWD} ssh -t -o StrictHostKeyChecking=no -p ${SSH_PORT} ${SSH_USR}@${ADDR} sudo snap run --gdbserver=:${GDB_PORT} rexroth-automationcore.control