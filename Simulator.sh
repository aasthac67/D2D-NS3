#!/usr/bin/env bash

# ARG1 $1 is simulationTime
# ARG2 $2 is optimalSinr
# ARG3 $3 is simulationInterval

value=0
iterations=($1/$3)
LteEnbPhyTxPower=30
optimalSinr=$2
stepfront=40
stepback=40


for (( c=0; c<$iterations; c++ ))
do  
   
	if [[ c -eq 0 ]]; 
	then
		./waf --run "scratch/ACN_171IT102_171IT109_171IT127 --simTime='$3'"
		value=$(<./scratch/sinr.txt)
		echo $value
	else
		value=$(<./scratch/sinr.txt)
		if [ $(echo "$value < $optimalSinr" | bc) -ne 0 ]; 
		then
			LteEnbPhyTxPower=`expr "$LteEnbPhyTxPower + $stepfront" | bc`
			echo $LteEnbPhyTxPower
			./waf --run "scratch/ACN_171IT102_171IT109_171IT127 --eNbPower='$LteEnbPhyTxPower' --simTime='$3'"
			stepback=($stepback/2)
		else
			LteEnbPhyTxPower=`expr "$LteEnbPhyTxPower - $stepback" | bc`
			echo $LteEnbPhyTxPower
			./waf --run "scratch/ACN_171IT102_171IT109_171IT127 --eNbPower='$LteEnbPhyTxPower' --simTime='$3'"
			stepfront=($stepfront/2)

		fi
	fi

done
  