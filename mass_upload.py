import subprocess
import time
import msvcrt
counter = 0

while True:
    start_time = time.time()
    counter += 1
    print("===================================")
    print("%d. feltoltes folyamatban... " % counter)
    result = subprocess.run(['pio', 'run', '-t', 'upload', '-s'], shell=True, capture_output=True, text=True)
    print(result.stdout)
    end_time = time.time()
    print("Futasi ido: %d mp." % (end_time-start_time))
    key = input("A folytatashoz nyomj le egy billenytyut. Kilepes q.\n")
    # key = msvcrt.getch()
    if key == 'q':
        break


