#!/bin/bash
#collect data from bmp280 board

helpFunction()
{
   echo "Usage: $0 -ip 192.168.0.123 -out datasave.dat"
   echo -e "\t-i     Name out file to append data"
   echo -e "\t-o     IP address of ESP device collectiing data"
   echo -e "\t-r     Number of seconds between measurements"
   echo
   exit 1 # Exit script after printing help
}

while getopts "i:o:r:" opt
do
   case "$opt" in
      i ) ip="$OPTARG" ;;
      o ) out="$OPTARG" ;;
      r ) sec="$OPTARG" ;;
      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
   esac
done

# Print helpFunction in case parameters are empty
if [ -z "$sec" ] 
  then sec=2
fi
if [ -z "$out" ] 
  then out="temp.dat" 
fi
if [ -z "$ip" ] 
  then helpFunction 
fi

# Begin script in case all parameters are correct
echo
echo "Data from $ip to be appendeded to $out every $sec seconds"
echo 
a=0
while true;
  do
     date +"%m-%d-%Y %T" > temp
     curl $ip?getData --output - 2> /dev/null >> temp
     if [ "$a" -gt 1 ]; 
     then
         cat temp |tr -s "\n" "\t" | tee -a $out 
         echo | tee -a $out 
     fi
     let a++
     sleep $sec;
  done


