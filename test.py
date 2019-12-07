import sys
sys.path.append(r'D:\Development\github\MidiKraft-synths-ci\builds\PyTschirp\Debug')

from pytschirp import Rev2Patch, Rev2

a = Rev2Patch()
r = Rev2()
b = a.Slop

p = r.loadSysex(R"D:\Christof\Music\ProphetRev2\VCM-Sound Set\VCM-Rev2-Soundset-U3-v1.syx")
