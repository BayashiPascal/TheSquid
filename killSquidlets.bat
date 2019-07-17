for pid in `lsof -n | grep LISTEN | grep squidlet | awk '{print $2}'`; do kill -INT $pid;  done
