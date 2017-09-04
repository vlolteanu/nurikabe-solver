set -e

mkdir -p puzzle
mkdir -p solpuz

wget http://www.logicgamesonline.com/nurikabe/archive.php?pid=$1 -O puzzle/$1.raw

extract()
{
	file=$1
	what=$2

	line=$(cat $file | grep "var $what =" | cut -d '"' -f 2)

	for i in $(seq 0 8)
	do
		off=$((9 * i))
		echo ${line:$off:8} 
	done
}

extract puzzle/$1.raw puzzle > puzzle/$1.puzzle
extract puzzle/$1.raw solpuz > solpuz/$1.sol

rm puzzle/$1.raw

