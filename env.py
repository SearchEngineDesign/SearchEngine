#!/usr/bin/python3


import subprocess

with open(".env", 'r') as env_file, open("run_script.sh", 'w') as run_script:
    
    run_script.write('#!/bin/bash\n')
    
    lines = env_file.readlines()
    for line in lines:
        line = line.strip()
        
        if not line:
            continue
        
        try: 
            if line[0] == '#':
                continue
        
            key, value = line.split('=')
            key = key.strip()
            value = value.strip()

            run_script.write(f"export {key}='{value}'\n")
            
            # os.environ[key] = value
            
            # print(f"SET {key} = {value}")

        except Exception as e:
            print("IT BE YOUR OWN")
    # run_script.write("make clean\n")
    # run_script.write("make\n")
    # run_script.write("./search")
    
    
    
    # print("Script run_script.sh created successfully.")
    # print("Use ./run_script.sh to run the search engine")


# subprocess.run(["chmod", "+x", "run_script.sh"])
# subprocess.run(["./run_script.sh"])
print("Environment script created successfully.")
print("Use source run_scripy.sh to set the environment variables")
print("Use ./search to run the search engine")