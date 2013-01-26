#!/bin/sh
export LD_LIBRARY_PATH=/home/ubuntu/pokerbots2013/pbots_calc-master/export/linux2/lib:$LD_LIBRARY_PATH
java -cp /home/ubuntu/pokerbots2013/pbots_calc-master/java/jnaerator-0.11-SNAPSHOT-20121008.jar:/home/ubuntu/pokerbots2013/pbots_calc-master/java/bin pbots_calc.Calculator $@