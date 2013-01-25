import re
import io
import sys
import os
import subprocess

files = subprocess.check_output(["find", "-name", "Casino_Day-8*"]).split('\n')
#print files
ball_list = ['bAllin', 'Bobbit', 'CALL', 'CallMeMaybe', \
             'DrBowserzard', 'Ganbatte', 'HandBanana', 'Poseidon', 'team1' ]

for file_path in files:
    m = re.match('.*Casino_Day-([0-9]?)_(.*?)_vs_(.*?).txt', file_path)
    day, p1, p2 = m.group(1), m.group(2), m.group(3)
    if (p1 not in ball_list) and (p2 not in ball_list):
        print "annotating: Day", day, "players:", p1, "vs.", p2
        os.system("./annotator.sh "+day+" "+p1+" "+p2)
