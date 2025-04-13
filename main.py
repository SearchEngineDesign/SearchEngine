import os
import subprocess
from time import sleep
import sys

from google.cloud import storage

os.environ["GOOGLE_APPLICATION_CREDENTIALS"]="$HOME/.config/gcloud/application_default_credentials.json"

from oauth2client.service_account import ServiceAccountCredentials

LOG_FILE = 'out'
SLEEP_INTERVAL = 30
MAX_CHUNKS = 100

def upload_to_bucket(blob_name, path_to_file, bucket_name):
    """ Upload data to a bucket"""
     
    # Explicitly use service account credentials by specifying the private key
    # file.
    storage_client = storage.Client.from_service_account_json(
        'creds.json')

    #print(buckets = list(storage_client.list_buckets())

    bucket = storage_client.get_bucket(bucket_name)
    blob = bucket.blob(blob_name)
    blob.upload_from_filename(path_to_file)
    
    #returns a public url
    return blob.public_url

def run():
    sys.stdout.reconfigure(encoding='utf-8')
    result = subprocess.Popen(
        ['./search', "./log/frontier/list", "./log/frontier/bloomfilter.bin", ">", LOG_FILE], 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE, 
        text=True
    )

    oldln = 0
    newln = 0
    
    while True:
        sleep(SLEEP_INTERVAL)
        newln = len(os.listdir("./log/chunks"))
        if newln == oldln:
            print("Process stalled. Restarting...")
            result.terminate()
            sleep(SLEEP_INTERVAL)
            if result.poll() is None: # if subprocess hasn't shut down
                result.kill()
            result = subprocess.Popen(
                ['./search', "./log/frontier/list", "./log/frontier/bloomfilter.bin", ">", LOG_FILE], 
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE, 
                text=True
            )
        elif newln > MAX_CHUNKS:
            print("Transferring chunks...")
            upload_to_bucket()
        oldln = newln




def main():
    run()

if __name__ == '__main__':
    main()
    