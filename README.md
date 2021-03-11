# teelog
tee like logger

yes | teelog -l 1000 -f 4 /tmp/tee.log

or to aquire exit status, use below method:

mkfifo /tmp/fifo-vsh
teelog-arm64 /tmp/vsH.log < /tmp/fifo-vsh &
${HOME}/${VSH} &> /tmp/fifo-vsh
RET=$?
