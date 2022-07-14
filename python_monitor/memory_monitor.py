import psutil
import time
import os
import sys
from csv import writer
from datetime import datetime

interval = 60 * 2

def append_list_as_row(file_name, list_of_elem):
  with open(file_name, 'a+', newline='') as csv_file:
    csv_writer = writer(csv_file)
    csv_writer.writerow(list_of_elem)

def get_pid(pid_file: str):
  try:
    with open(pid_file, "r") as f:
      pid = f.readlines()[0].rstrip()
      pid = int(pid)
      if psutil.pid_exists(pid):
        return pid
  except:
    pass
  return -1

if __name__ == "__main__":
  defichain_data_dir = sys.argv[1]
  pid_file = os.path.join(defichain_data_dir, "defid.pid")
  
  while True:
    pid = get_pid(pid_file)
    if pid != -1:
      p = psutil.Process(pid)
      mem_info = p.memory_info()
      rss = mem_info.rss
      kb = rss / 1024
      mb = int(kb / 1024)
      current_time = datetime.now()
      # print(f"{current_time}: {mb} MB")
      output_file = os.path.join(defichain_data_dir, f"memory_{pid}.csv")
      append_list_as_row(output_file, [current_time, mb])
    else:
      pass
      # print("PID not found..")

    time.sleep(interval)