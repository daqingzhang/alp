# list all shm info to debug
ipcs -m

# show one shm details info
ipcs -m -i 33062949

# remove a errornously left shm
ipcrm shm 33062949
