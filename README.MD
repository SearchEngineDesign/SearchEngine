EECS 498 Search Engine

To run the engine

1. Inititalize environment variables by defining them in .env and running env.py

A suitable config looks like. For each instance you'd want to run add the corresponding IP address and update the NODE_ID. 2 Nodes requires two different shells with identical configs except for NODE_ID. NODE_ID should be [0, NUM_NODES)


``` bash
NODE_IP0 = 127.0.0.1
NODE_IP1 = 127.0.0.1
NODE_ID = 0
NUM_NODES = 2
```

2. Run make


3. ./search