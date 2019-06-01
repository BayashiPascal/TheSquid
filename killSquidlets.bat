for pid in `ps | grep squidlet | awk '{print $1}'`; do kill -INT $pid;  done
