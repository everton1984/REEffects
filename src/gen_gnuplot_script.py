#!/usr/bin/python2
import config
import sys

cfg = dict()
if len(sys.argv) > 1:
    cfg = config.read_config(sys.argv[1])
else:
    cfg = config.read_config(config.DEFAULT_CONFIG_FILE)

scriptfname = "gscript"
if len(sys.argv) > 2:
    scriptfname = sys.argv[2]

prefix = cfg['output_prefix']
outdir = cfg['output_dir']

points = open(outdir + "/points_" + prefix + "_all")
size = len(points.readlines())
points.close()

script = "set terminal png size 4096,4096\n"
bigscript = "plot \"" + outdir + "/roads_" + prefix + "_all\" with lines "
for i in range(0,size):
    script = script + "set output \"" + outdir + "/chunks_" + prefix + "_" + str(i) + ".png\"\n"
    script = script + "plot \"" + outdir + "/points_" + prefix + "_" + str(i) + "\" with points, \"" + outdir + "/chunks_" + prefix + "_" + str(i) + "\" with lines\n"
    script = script + "set output \"" + outdir + "/chunks_raw_" + prefix + "_" + str(i) + ".png\"\n"
    script = script + "plot \"" + outdir + "/points_" + prefix + "_" + str(i) + "\" with points" 
    script = script + ",\"" + outdir + "/chunks_raw_" + prefix + "_" + str(i) + "\" with lines\n"
    bigscript = bigscript + ",\"" + outdir + "/points_" + prefix + "_" + str(i) + "\" with points, \"" + outdir + "/chunks_" + prefix + "_" + str(i) + "\" with lines"

gscript = open(scriptfname,"w")
gscript.write(script + "set output \"" + outdir + "/chunks_" + prefix + "_all.png\n" + bigscript)
gscript.close()
