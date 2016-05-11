#!/usr/bin/python2
prefix = "caucaia"
points = open("./out/points_" + prefix + "_all")
size = len(points.readlines())
points.close()

script = "set terminal png size 4096,4096\n"
bigscript = "plot \"./out/roads_" + prefix + "_all\" with lines "
for i in range(0,(size-1)):
    script = script + "set output \"./out/chunks_" + prefix + "_" + str(i) + ".png\"\n"
    script = script + "plot \"./out/points_" + prefix + "_" + str(i) + "\" with points, \"./out/chunks_" + prefix + "_" + str(i) + "\" with lines\n"
    script = script + "set output \"./out/chunks_raw_" + prefix + "_" + str(i) + ".png\"\n"
    script = script + "plot \"./out/points_" + prefix + "_" + str(i) + "\" with points" 
    #script = script + ", \"./out/roads_" + prefix + "_all\" with lines"
    script = script + ",\"./out/chunks_raw_" + prefix + "_" + str(i) + "\" with lines\n"
    bigscript = bigscript + ",\"./out/points_" + prefix + "_" + str(i) + "\" with points, \"./out/chunks_" + prefix + "_" + str(i) + "\" with lines"

gscript = open("gscript","w")
gscript.write(script + "set output \"./out/chunks_" + prefix + "_all.png\n" + bigscript)
gscript.close()
