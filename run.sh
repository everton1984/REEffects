#!/bin/zsh
cfg=$1
if [[ -z $cfg ]]; then
    echo "Please run with 'run.sh <config_file>'"
    exit
fi

echo "Do you want to recompile?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) echo "Building..."; build; break;;
        No ) break;;
    esac
done

echo "Do you want to calculate the neighborhoods?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) echo "Generating chunks...";bin/get_chunks $cfg; break;;
        No ) break;;
    esac
done

echo "Calculating index..."
src/calc_ire.py $cfg > ./tmp/results.csv
echo "Generating plots..."
src/gen_gnuplot_script.py $cfg ./tmp/gscript
gnuplot ./tmp/gscript
rm -f ./tmp/gscript
echo "Done!"

