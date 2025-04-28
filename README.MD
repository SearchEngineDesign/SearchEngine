EECS 498 Search Engine - Code Father

There are three principal processes involved in search engine. These are the crawling apparatus, the frontend, and the index server.

1. The crawling apparatus

    This is the most complex part of the project. It is ran via master.sh, which monitors the size and status of the crawling loop. Continuously running this loop creates index chunks indefinitely. The chunks are stored in the folder ./log/chunks.

    The process is run with nohup. Its output can be monitored via "tail -f nohup.out".

    When the process exits, its frontier and bloom filter are written out to a specified list file and a bloomfilter.bin file, respectively. The list file is the same file of endline-delineated urls that is specified on startup. master.sh uses log/frontier/list as the starting file.

2. The frontend

    Currently hosted locally, the frontend takes in queries and distributes them to the index servers. Once it receives data back from the servers (in the format "{url}\t{score}\n), it ranks the urls according to their score, ignoring duplicates. It then outputs a list of urls. 

    The frontend is run via the "run_server.sh" script in ./frontend.

3. The index server

    Each of our 10 virtual machines runs its own server, which seek index chunks based on the queries they receive. The results are ranked, and then sent over a socket to the frontend.

    The servers are run through the "run_indexserver.sh" script in ./frontend. Because the IP addresses of the Google Cloud VMs are not dynamic, we have to update the addresses in the frontend code occasionally.