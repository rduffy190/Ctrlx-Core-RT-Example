#!/bin/bash
set -euo pipefail
source /home/riley/dev/RealTimeAppTemplate/parts/bundle/run/environment.sh
set -x
cp --archive --link --no-dereference . "/home/riley/dev/RealTimeAppTemplate/parts/bundle/install"
