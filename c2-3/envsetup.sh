function add_sauce()
{
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$(pwd)

	echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
}

add_sauce
