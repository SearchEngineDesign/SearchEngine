import os



with open(".env", 'r') as f:
    lines = f.readlines()
    for line in lines:
        line = line.strip()
        
        if not line:
            continue
        
        try: 
            if line[0] == '#':
                continue
        
            key, value = line.split('=')

            os.environ[key] = value
            print(f"SET {key} = {value}")

        except Exception as e:
            print("IT BE YOUR OWN")
