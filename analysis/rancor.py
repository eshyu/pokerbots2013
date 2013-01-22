import re
import io
import sys
import os
import subprocess

files = subprocess.check_output(["find", "-name", "Casino_Day-7*"]).split('\n')
#print files

for file_path in files:
    m = re.match('.*Casino_Day-([0-9]?)_(.*?)_vs_(.*?).txt', file_path)
    day, p1, p2 = m.group(1), m.group(2), m.group(3)
    print "analyzing: Day", day, "players:", p1, "vs.", p2
    os.system("./analysis.sh "+day+" "+p1+" "+p2)
