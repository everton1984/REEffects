#!/usr/bin/python2
DEFAULT_CONFIG_FILE = "./cfg/config"

def read_config(fname):
    configFile = open(fname)
    lines = configFile.readlines()
    configFile.close()
    cfg = dict()

    for l in lines:
        d = l.split('=')
        cfg[d[0].lower().strip()] = d[1].strip()

    return cfg


