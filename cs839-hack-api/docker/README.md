To build, open a terminal in this directory and execute...
`docker build -f Dockerfile .. -t ctnelson1997/cs839-hack`

To run, open a terminal in any directory and execute...
`docker run -v /cs839/hack:/secrets --restart=always -d -p 39839:39839 --name cs839-hack ctnelson1997/cs839-hack:latest`
Note that you can change the beginning port number to any open port.

To view logs, open a terminal in any directory and execute...
`docker ps` (if the container is down, execute `docker ps -a`)
Find the container id (in this case, for `cs571-cs839-hack`) and execute...
`docker logs {CONTAINER_ID_HERE}`