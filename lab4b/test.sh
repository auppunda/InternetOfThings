./lab4b --log="f.txt" --period=2 --scale=F << 'EOF'
PERIOD=5
PERIOD=5
PERIOD=5
PERIOD=5
PERIOD=5
PERIOD=5
PERIOD=5
SCALE=F
SCALE=C
STOP
START
LOG starting
OFF
EOF

for c in SCALE=F PERIOD=5 START STOP OFF SHUTDOWN "LOG starting"
do
	grep "$c" f.txt > /dev/null
	if [ $? -ne 0 ]
	then
		echo "DID NOT LOG $c command"
		let errors+=1
	else
		echo "    $c ... RECOGNIZED AND LOGGED"
	fi
done

rm -f f.txt

#bogus test
./lab4b --log="f.txt" --period=2 << 'EOF'
BOO
OFF
EOF


for c in OFF SHUTDOWN
do
        grep "$c" f.txt > /dev/null
        if [ $? -ne 0 ]
        then
                echo "DID NOT LOG $c command"
                let errors+=1
        else
                echo "    $c ... RECOGNIZED AND LOGGED"
        fi
done

egrep '[0-9][0-9]:[0-9][0-9]:[0-9][0-9] SHUTDOWN' f.txt
if [ $? -eq 0 ]
then
	echo "SHUTDOWN FORMATING IS GOOD"
else 
	echo "REVISE FORMATTING"
fi 

egrep '[0-9][0-9]:[0-9][0-9]:[0-9][0-9] [0-9]+\.[0-9]\>' f.txt
if [ $? -eq 0 ]
then
        echo "Temperature FORMATING IS GOOD"
else 
        echo "REVISE FORMATTING"
fi 
rm -f f.txt

# tests if f.txt is also created so it checks the logging feature



