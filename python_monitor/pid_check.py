import psutil
import time
import os

interval = 60 * 10

def start_defichain_node(script_path: str):
  os.system(script_path)

def is_running(pid_file: str):
  try:
    with open(pid_file, "r") as f:
      pid = f.readlines()[0].rstrip()
      pid = int(pid)
      if psutil.pid_exists(pid):
        return True
  except:
    return False
  return False

if __name__ == "__main__":
  defichain_data_dir = sys.argv[1]
  defichain_start_script = sys.argv[2]
  pid_file = os.path.join(defichain_data_dir, "defid.pid")

  while True:
    time.sleep(interval)
    active = is_running(pid_file)
    if not active:
      print("Restart node..")
      start_defichain_node(defichain_start_script)
      time.sleep(10)
    else:
      print("Node is running..")