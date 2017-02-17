#! /bin/sh
### BEGIN INIT INFO
# Provides:          hyperion-remote
# Required-Start:    $local_fs $network
# Required-Stop:     $local_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: hyperion-remote
# Description:       hyperion-remote server
### END INIT INFO
python /home/hs.py &
